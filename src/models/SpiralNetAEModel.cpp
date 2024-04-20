//======================================================================================================================
// Copyright (c) by Fabian Kemper 2024
//
// This work is licensed under a
// Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//
// You should have received a copy of the license along with this
// work. If not, see <http://creativecommons.org/licenses/by-nc-sa/4.0/>.
//
//======================================================================================================================

// torch has to be the first include, to prevent namespace clash with pmp::Scalar
#include <fmt/format.h>
#include <imgui.h>
#include <pmp/stop_watch.h>
#include <pmp/io/io.h>
#include <torch/torch.h>
#include <torch/nn/modules/loss.h>

#include "SpiralNetAEModel.h"

#include "Globals.h"
#include "utils/io/filesystem_utils.h"
#include "utils/io/ndarray_io.h"
#include "utils/io/pmp_io.h"
#include "utils/name_utils.h"

//======================================================================================================================

using SurfaceMesh = pmp::SurfaceMesh;

//======================================================================================================================


SpiralNetAEModel::SpiralNetAEModel()
{
    if (torch::cuda::is_available()) {
        _device = torch::kCUDA;
        std::cout << "[Info] Device: CUDA\n";
    } else if (torch::mps::is_available()) {
        // if pytorch_geometric support kMPS in the future
//        _device = torch::kMPS;
        _device = torch::kCPU;
        std::cout << "[Info] Device: CPU (MPS available, but incompatible)\n";
    } else {
        std::cout << "[Info] Device: CPU\n";
    }
}

// ---------------------------------------------------------------------------------------------------------------------

auto SpiralNetAEModel::get_model_filename() -> std::string
{
    std::string mesh_name = NameUtils::mesh_type_str(_mesh_type);
    auto filename = fmt::format("{}.zip", mesh_name);
    auto result = std::filesystem::path(globals::model_dir) / "spiral" / filename;
    return result.string();
}

// ---------------------------------------------------------------------------------------------------------------------

auto SpiralNetAEModel::load_model(const std::string& filename) -> void
{
    _model_loaded = false;
    _model_version = 0;

    if (!FilesystemUtils::file_exists(filename)) {
        std::cerr << "Could not find model " << filename << '\n';
        return;
    }

    libz::ZipArchive zip_archive(filename);
    try {
        zip_archive.open(libz::ZipArchive::ReadOnly);

        // extract meta data dictionary
        std::stringstream meta_buffer;
        extract_to_buffer(zip_archive, "meta.json", meta_buffer);
        _meta = json::parse(meta_buffer);

        // load model version
        if (_meta.contains("model_version")) {
            _model_version = _meta["model_version"].get<int>();
        }
        std::cout << "Model version: " << _model_version << '\n';
        if (_model_version < 2) {
            throw std::runtime_error("Model version < 2 not supported.");
        }

        // space in extraction buffer
        std::stringstream buffer {};

        // extract model
        extract_to_buffer(zip_archive, "autoencoder.pt", buffer);
        _model = torch::jit::load((std::istream &) buffer, _device);
        _model.eval();
        _model_loaded = true;

        // load mean and std
        extract_to_buffer(zip_archive, "mean.dat", buffer);
        _mean = NDArray::read_vector_f(buffer);
        extract_to_buffer(zip_archive, "std.dat", buffer);
        _std = NDArray::read_vector_f(buffer);
        // replace small standard deviation by 0.0
        _std = _std.unaryExpr([](float value) { return abs(value) > 1.0e-10F ? value : 1.0F; });
        if (_mean.size() != _std.size()) {
            std::cerr << "[Error] mean.size() != std.size().\n";
        }

        // load mean skeleton and skin
        extract_to_buffer(zip_archive, "skel.obj", buffer);
        read_obj_buffer(_skel, buffer);
        extract_to_buffer(zip_archive, "skin.obj", buffer);
        read_obj_buffer(_skin, buffer);

        std::cout << "Model loaded." << '\n';
    } catch (std::runtime_error& exception) {
        std::cerr << exception.what() << '\n';
    }
}

// ---------------------------------------------------------------------------------------------------------------------

auto SpiralNetAEModel::inference_available() const -> bool
{
    return _model_loaded;
}

// ---------------------------------------------------------------------------------------------------------------------

auto SpiralNetAEModel::latent_dimensions() -> int
{
    if (_model_loaded && _meta.contains("latent_meta")) {
        return static_cast<int> (_meta["latent_meta"].size());
    }

    return BaseModel::latent_dimensions();
}

// ---------------------------------------------------------------------------------------------------------------------

auto SpiralNetAEModel::latent_channels(int dimension) -> int
{
    if (_meta.contains("latent_meta")
        && _meta["latent_meta"].contains("latent_channels_skel")
        && _meta["latent_meta"].contains("latent_channels_skin"))
    {
        if (dimension == 0) {
            return _meta["latent_meta"]["latent_channels_skel"].get<int>();
        }
        if (dimension == 1) {
            return _meta["latent_meta"]["latent_channels_skin"].get<int>();
        }
    }
    return 0;
}

// ---------------------------------------------------------------------------------------------------------------------

auto SpiralNetAEModel::latent_dimension_name(int dimension) -> std::string
{
    if (dimension == 0) {
        return {"Skeleton"};
    }
    if (dimension == 1) {
        return {"Soft Tissue"};
    }
    return {};
}

// ---------------------------------------------------------------------------------------------------------------------

auto SpiralNetAEModel::latent_channel_name(int dimension, int channel) -> std::string {
    if (_model_version >= 2 && _meta.contains("named_parameters")) {
        json parameters;
        if (dimension == 0 && _meta["named_parameters"].contains("skel")) {
            parameters = _meta["named_parameters"]["skel"];
        }
        if (dimension == 1 && _meta["named_parameters"].contains("skin")) {
            parameters = _meta["named_parameters"]["skin"];
        }

        if (parameters.contains(std::to_string(channel))) {
            return parameters[std::to_string(channel)];
        }
    }

    return BaseModel::latent_channel_name(dimension, channel);
}

// ---------------------------------------------------------------------------------------------------------------------

auto SpiralNetAEModel::_inference_torch(const ArrayXf& weights) -> ArrayXf
{
    ArrayXf result = BaseModel::inference(weights);

    if (weights.size() != latent_channels_sum()) {
        std::cerr << "_inference_torch: Dimensions do not match. weights="
                  << weights.size() << " latent_dim=" << latent_channels_sum() << '\n';
        throw std::runtime_error("ERROR");
    }

    try {
        // convert to right format
        auto options = torch::TensorOptions().dtype(torch::kFloat32);
        // tensor input on cpu
        torch::Tensor input_t = torch::from_blob((void *) weights.data(), {1, weights.size()}, options);
        // move to gpu (if available)
        input_t = input_t.to(_device);
        // create interface values from torch tensor (on gpu)
        torch::jit::IValue input = input_t;

        // inference
        torch::jit::IValue outputs = _model.run_method("decoder", input);

        at::Tensor output_tensor = outputs.toTensor();
        // bring back to cpu the result
        output_tensor = output_tensor.view({ 1, -1 }).to(at::DeviceType::CPU); // one batch ...

        // copy eigen memory
        result = Eigen::Map<VectorXf> { output_tensor.data_ptr<float>(), output_tensor.size(1) };
    } catch (c10::Error& error) {
        std::cerr << error.what() << '\n';
        return {};
    }
    return result;
}

// ---------------------------------------------------------------------------------------------------------------------

auto SpiralNetAEModel::_fit_skin(ArrayXf& target) -> ArrayXf
{
    auto latent_variables = ArrayXf { latent_channels_sum() };
    latent_variables.setZero();

    if (target.size() != static_cast<long>(get_mean_skin().n_vertices()) * 3) {
        std::cerr << "Cannot fit! target_vertices=" << target.size() / 3 << " skin_vertices=" << get_mean_skin().n_vertices() << '\n';
        return latent_variables;
    }

    if (!_model_loaded) {
        return latent_variables;
    }

    // convergence parameters
    int max_steps = 100;
    double max_loss_degradation = 10.0e-2; // percentage!
    double min_loss_improvement = 0.15e-2; // percentage!
    double learning_rate = 7.5e-2;

    // torch options, with and without gradient
    auto no_grad = torch::TensorOptions().dtype(torch::kFloat32);
    auto with_grad = torch::TensorOptions().dtype(torch::kFloat32).requires_grad(true).device(_device);


    // cast to float ...
    ArrayXf target_f = target;

    if (target_f.size() > _mean.size()) {
        std::cerr << "[Error] More values to predict than available in model. Cannot fit skin.\n";
    }

    // resize target (reserve dummy skel as 0) if only skin is given
    if (target_f.size() < _mean.size()) {
        ArrayXf target_tmp{};
        target_tmp.resize(_mean.size());
        target_tmp.setZero();
        target_tmp.block(_mean.size() - target_f.size(), 0, target_f.size(), 1) = target_f;
        target_f = target_tmp;
    }

    // Loss to L1 vertex distance
    ArrayXf target_vert_space = (target_f - _mean);
    // register tensor with data, move it to gpu (if available)
    auto torch_target = torch::from_blob(target_vert_space.data(), {target_f.size() }, no_grad).to(_device);
    torch::Tensor current_fit;

    // initialize latent variables as zero
    auto latent_fit = torch::zeros({1, static_cast<long> (latent_variables.size())}, with_grad);
    latent_fit = latent_fit.to(_device);

    // momentum and weight decay for adam
    auto adam_options = torch::optim::AdamOptions(/*lr=*/learning_rate);
    adam_options = adam_options.betas(std::make_tuple(0.5, 0.5));
    adam_options = adam_options.weight_decay(_weight_decay);

    // optimizer for latent code
    auto optimizer = torch::optim::Adam(
        std::vector<at::Tensor>{latent_fit},
        adam_options
    );

    auto n_entries_skel = get_mean_skel().n_vertices() * 3;
    // MSELoss = mean squared vertex distance , L1Loss = mean absolute error
    auto loss_func = torch::nn::L1Loss();
    double best_skin_loss = 10e10;
    at::Tensor target_vert_mc;
    // current fit in vertex space (mean centered)
    at::Tensor current_fit_vert_mc;

    // standard deviation for vertex positions
    auto std_dev = torch::from_blob(_std.data(), {_std.size()}, no_grad).index({torch::indexing::Slice(n_entries_skel, torch::indexing::None)});
    std_dev = std_dev.to(_device);

    for (auto step = 0; step < max_steps; ++step) {
        optimizer.zero_grad();
        current_fit = _model.run_method("decoder", latent_fit).toTensor();
        current_fit = current_fit.reshape({current_fit.size(1) * current_fit.size(2)});

        current_fit_vert_mc = current_fit.index({torch::indexing::Slice(n_entries_skel, torch::indexing::None)}) * std_dev;
        target_vert_mc = torch_target.index({torch::indexing::Slice(n_entries_skel, torch::indexing::None)});
        auto loss = loss_func(current_fit_vert_mc, target_vert_mc);

        // for statistic
        auto loss_l2 = torch::nn::MSELoss()(current_fit_vert_mc, target_vert_mc);
        std::cout << "Step " << std::setw(2) << step << " Loss (L1): " << loss.item() << ", Loss (L2): " << loss_l2.item() << '\n';

        double loss_improvement = best_skin_loss - loss.item().toDouble();
        double loss_degradation = std::max(loss.item().toDouble() - best_skin_loss, 0.0);

        // track improvement
        if (loss.item().toDouble() < best_skin_loss) {
            best_skin_loss = loss.item().toDouble();
        }

        double rel_degradation = loss_degradation / best_skin_loss;
        double rel_improvement = std::abs(loss_improvement / best_skin_loss);

        if (rel_degradation > max_loss_degradation) {
            std::cout << "Stop fitting: Gradient increased too much.\n";
            break;
        }

        // perform optimization step
        loss.backward();
        optimizer.step();

        // the improvement was not good, this was the last step
        if ((rel_improvement < min_loss_improvement) && loss_improvement > 0.0) {
            std::cout << "Stop fitting: No improvement.\n";
            break;
        }
    }

    // vector distance (x_i,pred - x_i)^2
    // for every three elements, sum =  x^2 + y^2 + z^2
    // square root of sum = distance
    auto x_dist_vec = (current_fit_vert_mc - target_vert_mc).square().reshape({-1, 3}).sum(1).sqrt();
    auto mean_distance = x_dist_vec.mean(0);

    double error_mm = mean_distance.item().toDouble();
    std::cout << "Loss in mm " << error_mm * 1000.0 << " mm\n";

    // convert to final parameters
    latent_fit = latent_fit.contiguous().to(torch::DeviceType::CPU);
    std::vector<float> result (latent_fit.data_ptr<float>(), latent_fit.data_ptr<float>() + latent_fit.numel());

    // copy to latent variables
    std::memcpy(latent_variables.data(), result.data(), latent_variables.size() * sizeof(float));
    return latent_variables;
}

// ---------------------------------------------------------------------------------------------------------------------

auto SpiralNetAEModel::inference(const ArrayXf& weights) -> ArrayXf
{
    if (_model_loaded) {
        // perform torch inference for weights
        ArrayXf result = _inference_torch(weights);

        if (result.size() != _mean.size() || result.size() != _std.size()) {
            std::cout << "[Error] inference: result.size=" << result.size() << ", mean.size=" << _mean.size() << '\n';
            return BaseModel::inference(weights);
        }

        // add mean and scale by std_dev
        result = result * _std + _mean;

        // use only delta of target skin
        long skel_size = static_cast<long>(_skel.n_vertices()) * 3;
        if (_inference_mode == FITTING_DELTA) {
            if (result.size() == (skel_size + _target_skin.size())
                && _target_skin_fit.size() == _target_skin.size())
            {
                // predicted skin of current latent input // == f(z)
                ArrayXf pred_skin = result(Eigen::seqN(skel_size, _target_skin.size()));

                // g = decoder
                // difference to fit "best fit" of scanned person
                // delta = g(z) - g(~z)
                ArrayXf delta = pred_skin - _target_skin_fit.array();

                // add delta to target skin (loaded)
                // result is scan input (=x) + g(z) - g(~z)
                result(Eigen::seqN(skel_size, _target_skin.size())) = _target_skin + delta;
            } else {
                std::cerr << "FITTING_PREDICTION dimension mismatch.\n";
            }
        }

        return result;
    }
    // no model loaded
    return BaseModel::inference(weights);
}

// ---------------------------------------------------------------------------------------------------------------------

auto SpiralNetAEModel::set_mesh_type(MeshType mesh_type) -> void
{
    BaseModel::set_mesh_type(mesh_type);
    load_model(get_model_filename());
}

// ---------------------------------------------------------------------------------------------------------------------

auto SpiralNetAEModel::get_mean_skel() -> SurfaceMesh
{
    return _skel;
}

// ---------------------------------------------------------------------------------------------------------------------

auto SpiralNetAEModel::get_mean_skin() -> SurfaceMesh
{
    return _skin;
}

// ---------------------------------------------------------------------------------------------------------------------

auto SpiralNetAEModel::set_target_skin(ArrayXf& target_skin) -> void
{
    _target_skin = target_skin;
}

// ---------------------------------------------------------------------------------------------------------------------

auto SpiralNetAEModel::fit_target() -> void
{
    _target_latent = _fit_skin(_target_skin);

    // perform "base" mesh inference
    InferenceMode _old_mode = _inference_mode;
    _inference_mode = InferenceMode::NORMAL;
    long skel_size = static_cast<long>(_skel.n_vertices()) * 3;
    _target_skin_fit = inference(_target_latent)(Eigen::seqN(skel_size, _target_skin.size()));
    _inference_mode = _old_mode;
}

// ---------------------------------------------------------------------------------------------------------------------

auto SpiralNetAEModel::get_latent_fit() -> ArrayXf
{
    return _target_latent;
}

// ---------------------------------------------------------------------------------------------------------------------

//======================================================================================================================
