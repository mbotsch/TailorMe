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

#ifndef TAILORME_VIEWER_SPIRALNETAEMODEL_H
#define TAILORME_VIEWER_SPIRALNETAEMODEL_H

// before pmp
#include <torch/torch.h>
#include <torch/cuda.h>
#include <torch/script.h>

#include "BaseModel.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

// ---------------------------------------------------------------------------------------------------------------------

class SpiralNetAEModel : public BaseModel
{
  protected:
    // model
    torch::jit::Module _model {};
    json _meta {};
    bool _model_loaded = false;
    // vertex mean and stddev in xyz format
    ArrayXf _mean {};
    ArrayXf _std {};
    int _model_version = 0;

    // debugging parameters
    float _weight_decay = 7.5e-5;

    // use at::DeviceType::CPU to be safe
    // Todo: Change to cuda
    torch::Device _device = torch::kCPU;

    // meshes
    pmp::SurfaceMesh _skel {};
    pmp::SurfaceMesh _skin {};

    auto get_model_filename() -> std::string;

    // load existing model from disk
    auto load_model(const std::string& filename) -> void;

    // inference torch model
    auto _inference_torch(const ArrayXf& weights) -> ArrayXf;

    // fit latent variables with given skin
    auto _fit_skin(ArrayXf& target) -> ArrayXf;

    // fitting target
    ArrayXf _target_skin{};
    ArrayXf _target_skin_fit{};
    ArrayXf _target_latent{};

  public:
    SpiralNetAEModel();
    ~SpiralNetAEModel() override = default;

    // is model loaded?
    [[nodiscard]]
    auto inference_available() const -> bool override;

    // number of tabs for latent channels
    auto latent_dimensions() -> int override;
    // sum of latent parameters
    auto latent_channels(int dimension) -> int override;
    // name of latent dimensions
    auto latent_dimension_name(int dimension) -> std::string override;
    // name of channel
    auto latent_channel_name(int dimension, int channel) -> std::string override;

    // calls evaluate internally
    auto inference(const ArrayXf& weights) -> ArrayXf override;

    // load model when mesh type is set
    auto set_mesh_type(MeshType mesh_type) -> void override;

    // get mean mesh
    auto get_mean_skel() -> pmp::SurfaceMesh override;
    auto get_mean_skin() -> pmp::SurfaceMesh override;

    // set a fitting skin target (copy values into model)
    // setting x
    auto set_target_skin(ArrayXf& target_skin) -> void override;
    // perform fitting (this is subject to change)
    // this should set the "best fit" skin internally for delta changes
    // setting z
    auto fit_target() -> void override;
    // get fitted values for latent variables
    auto get_latent_fit() -> ArrayXf override;
};

// ---------------------------------------------------------------------------------------------------------------------

#endif // TAILORME_VIEWER_SPIRALNETAEMODEL_H
