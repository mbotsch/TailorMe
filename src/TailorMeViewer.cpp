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

#include <imgui.h>
#include <ImGuiFileDialog.h>

#include <pmp/io/io.h>

#include "Globals.h"
#include "TailorMeViewer.h"

#include "algorithms/MeshIntersection.h"
#include "models/SpiralNetAEModel.h"

#include "mesh_massage/post_proc_face_mirror.h"
#include "mesh_massage/post_proc_smoothing.h"

#include "utils/io/filesystem_utils.h"

//======================================================================================================================

// golden ratio 0.68
#define SLIDER_WIDTH 0.75F

//======================================================================================================================

using namespace pmp;

//======================================================================================================================

TailorMeViewer::TailorMeViewer(const char *title, int width, int height, bool show_gui)
    : TrackballViewer(title, width, height, show_gui)
{
    // setup draw modes
    clear_draw_modes();
    add_draw_mode("Smooth Shading");
    add_draw_mode("Hidden Line");
    add_draw_mode("Texture");

    // set default draw mode to texture (bone and skin material caps)
    set_draw_mode("Texture");

    // create bounding box for humans
    update_bb();

    // load default model & mesh
    set_model(ModelType::MODEL_SPIRAL_AE);
    set_mesh(MeshType::MESH_MALE);

    // add postprocessing
    _post_processing_filters.emplace_back(new PostProcessingFaceMirror());
    _post_processing_late_filters.emplace_back(new PostProcessingSmoothing());
}

//----------------------------------------------------------------------------------------------------------------------

void TailorMeViewer::update_bb()
{
    set_scene(pmp::Point{0.0F, 0.85F, 0.1F}, 1.00F);
    update_meshes();
}

//----------------------------------------------------------------------------------------------------------------------

/**
 * Call this function to update open_gl vertex, normal and triangle buffers
 */
void TailorMeViewer::update_meshes()
{
    if (_mesh != nullptr) {
        _mesh->update_meshes();
    }
}

//----------------------------------------------------------------------------------------------------------------------

void TailorMeViewer::do_processing()
{
    _optimization_suitable = _frames_without_user_interaction > 10;

    if (_optimization_required && _optimization_suitable)
    {
        if (_post_processing_enabled)
        {
            perform_post_processing();
            // visual swap
            _opacity_bone = 1.0F;
            _opacity_skel = 0.0F;
        }
    }

    if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
    {
        _frames_without_user_interaction++;
    }

    // Clamp so it doesn't overflow eventually
    if (_frames_without_user_interaction > 240)
    {
        _frames_without_user_interaction = 240;
    }
}

//-----------------------------------------------------------------------------

void TailorMeViewer::process_imgui()
{
    process_imgui_model();
    process_imgui_weights();
    process_imgui_fitting();
}


//----------------------------------------------------------------------------------------------------------------------

void TailorMeViewer::draw(const std::string& drawMode)
{
    if (!_show_target_mesh) {
        // show mesh from model
        if (_model != nullptr && _model->get_marked_for_inference()) {
            generate_meshes();
        }
        if (_mesh != nullptr) {
            _mesh->set_alpha(_opacity_bone, _opacity_skel, _opacity_skin);
            _mesh->draw(projection_matrix_, modelview_matrix_, drawMode);
        }
    } else {
        // show target mesh
        _target_skin.draw(projection_matrix_, modelview_matrix_, drawMode);
    }
}

//----------------------------------------------------------------------------------------------------------------------

/**
 * Calculates new positions by model for each model
 */
void TailorMeViewer::generate_meshes()
{
    // update plane points
    if (_mesh != nullptr && _model != nullptr && _model->inference_available()) {
        // guard to check for out of range writes
        if (_model->latent_channels_sum() != static_cast<long> (_latent_variables.size())) {
            std::cerr << "ERROR: Latent mismatch.\n";
            return;
        }
        // scale by X times std
        ArrayXf scaled_latent = _latent_variables * powf(WEIGHT_MAGNITUDE_BASE, _weight_magnitude);

        // inference points
        VectorXf points = _model->inference(scaled_latent);

        // set points for meshes
        _mesh->update_mesh_points(points);

        // postprocessing
        if (_post_processing_enabled) {
            auto* skel = _mesh->get_skel();
            auto* skin = _mesh->get_skin();
            if (skel != nullptr && skin != nullptr) {
                for (auto& filter : _post_processing_filters) {
                    filter->postprocess(skel, skin);
                }
            }
        }

        if (_inference_mode_delta && (_mesh_type == MESH_MALE || _mesh_type == MESH_FEMALE))
        {
            _mesh_stitcher.stitch();
        }

        _optimization_required = true;
        _frames_without_user_interaction = 0;
        _opacity_bone = 0.0F;
        _opacity_skel = 1.0F;

    } else {
        std::cerr << "Try to inference, but model not ready.\n";
    }

    update_meshes();
}

//----------------------------------------------------------------------------------------------------------------------

void TailorMeViewer::process_imgui_model()
{
    if (ImGui::CollapsingHeader("Model", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Mesh opacity");
        ImGui::PushItemWidth(ImGui::GetWindowWidth() * SLIDER_WIDTH);
        ImGui::SliderFloat("Skin##SkinOpacity", &_opacity_skin, 0.0F, 1.0F);
        ImGui::PopItemWidth();

        //ImGui::SameLine();
        if (ImGui::Button("0.5##alpha50")) {
            _opacity_skin = 0.50F;
        }
        ImGui::SameLine();
        if (ImGui::Button("0.75##alpha75")) {
            _opacity_skin = 0.75F;
        }
        ImGui::SameLine();
        if (ImGui::Button("1.0##alpha100")) {
            _opacity_skin = 1.00F;
        }

        ImGui::Spacing();
        ImGui::Text("Mesh");
        if (ImGui::Button("Male##Mesh")) {
            set_mesh(MESH_MALE);
        }
        ImGui::SameLine();
        if (ImGui::Button("Female##Mesh")) {
            set_mesh(MESH_FEMALE);
        }
    }
    ImGui::Spacing();
}

//----------------------------------------------------------------------------------------------------------------------

/** Show weight sliders for imgui **/
auto TailorMeViewer::process_imgui_weights() -> void
{
    bool force_mesh_inference = false;

    if (ImGui::CollapsingHeader("Latent Variables", ImGuiTreeNodeFlags_DefaultOpen)) {
        // show or hide unnamed latent parameters
        ImGui::Checkbox("Show un-named sliders", &_show_unnamed_sliders);
        ImGui::Spacing();

        if (_model != nullptr) {
            bool use_tabs = false;
            bool tab_bar_open = false;

            if (_latent_variables.size() != _model->latent_channels_sum()) {
                _latent_variables.resize(_model->latent_channels_sum());
                _latent_variables.setZero();
            }

            // tab splitting logic (... backward compatible for now)
            int current_tab = -1; // which tab is generated
            int current_tab_size = 0; // capacity for current tab
            int channel_offset = 0; // channels on previous tabs
            bool active_tab = false;
            bool prev_channel_has_name = false; // last channel had name

            ImGui::PushItemWidth(ImGui::GetWindowWidth() * SLIDER_WIDTH);
            //ImGui::PushItemWidth(ImGui::GetWindowContentRegionMax()[0] * 0.65f);

            // use tabs?
            if (_model->latent_dimensions() > 1) {
                use_tabs = true;
                tab_bar_open = ImGui::BeginTabBar("Latent##tabs", ImGuiTabBarFlags_None);
            }

            // iterate over channels and create tabs if needed
            for (long channel_idx = 0; channel_idx < _latent_variables.size(); ++channel_idx) {

                // switch to next page
                if ((static_cast<int>(channel_idx) - channel_offset >= current_tab_size)
                    && (static_cast<int> (current_tab) < _model->latent_dimensions()))
                {
                    current_tab++;
                    channel_offset += current_tab_size;
                    current_tab_size = _model->latent_channels(current_tab);

                    // previous tab was active, so close tab now
                    if (active_tab) {
                        ImGui::EndTabItem();
                    }
                    if (use_tabs) {
                        active_tab = ImGui::BeginTabItem(_model->latent_dimension_name(current_tab).c_str());
                    }

                    // first run
                    if (active_tab) {
                        ImGui::Text("Latent parameters: %s", _model->latent_dimension_name(current_tab).c_str());
                    }
                }

                // for each slider
                if (active_tab) {
                    // slider caption (if given)
                    auto channel_name =
                        _model->latent_channel_name(current_tab, static_cast<int>(channel_idx - channel_offset));
                    if (not channel_name.empty()) {
                        ImGui::Text("%s", channel_name.c_str());
                        prev_channel_has_name = true;
                    } else if (prev_channel_has_name) {
                        prev_channel_has_name = false;
                        ImGui::Spacing();
                    } else {
                        prev_channel_has_name = false;
                    }

                    // slider label (identifier)
                    std::string slider_label =
                        std::to_string(channel_idx) + std::string("##Lat") + std::to_string(channel_idx);

                    // show all sliders if enabled
                    if (not channel_name.empty() || _show_unnamed_sliders) {
                        if (ImGui::SliderFloat(slider_label.data(), &_latent_variables[channel_idx], -1.0F, 1.0F)) {
                            force_mesh_inference = true;
                        }
                    }
                }
            }

            // run if last tab was active
            if (active_tab) {
                ImGui::EndTabItem();
            }

            if (use_tabs && tab_bar_open) {
                ImGui::EndTabBar();
            }
        }

        // basic tab structure
        // ImGui::BeginTabBar("NAME");
        // if (ImGui::BeginTabItem("Title")) {
        //     ...
        //     ImGui::EndTabItem();
        // }
        // ImGui::EndTabBar();


        // reset weights
        ImGui::Spacing();
        if (ImGui::Button("Reset")) {
            for (auto& latent_value : _latent_variables) {
                latent_value = 0.0F;
            }
            _weight_magnitude = 0.0F;
            force_mesh_inference = true;
        }

        ImGui::SameLine();
        if (ImGui::Button("Random##weights")) {
            std::normal_distribution normal_dist {0.0, 0.4};
            std::random_device random_dev{};
            std::mt19937 random_gen{random_dev()};

            for (auto& weight : _latent_variables) {
                weight = static_cast<float>(normal_dist(random_gen));
            }
            force_mesh_inference = true;
        }
        ImGui::Spacing();

        // scale factor (logarithmic)
        std::ostringstream exponent_stream;
        exponent_stream << "Current weight factor " << powf(WEIGHT_MAGNITUDE_BASE, _weight_magnitude);
        std::string exponent_string = exponent_stream.str();
        ImGui::Text("%s", exponent_string.data());
        ImGui::PopItemWidth();

        // rescale weights
        if (ImGui::Button("1 std")) {
            rescale_weight_magnitude(0.0F); // // log_10(1)
            force_mesh_inference = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("1.5 std")) {
            rescale_weight_magnitude(0.17609F); // log_10(1.5)
            force_mesh_inference = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("2 std")) {
            rescale_weight_magnitude(0.30103F); // // log_10(2)
            force_mesh_inference = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("2.5 std")) {
            rescale_weight_magnitude(0.39794F); // log_10(1.5)
            force_mesh_inference = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("3 std")) {
            rescale_weight_magnitude(0.477121F);
            force_mesh_inference = true;
        }
        ImGui::Spacing();
    }

    // if any slider changed, regenerate mesh
    if (force_mesh_inference) {
        generate_meshes();
    }
}

//----------------------------------------------------------------------------------------------------------------------

auto TailorMeViewer::process_imgui_fitting() -> void
{
    if (ImGui::CollapsingHeader("Model fitting", ImGuiTreeNodeFlags_DefaultOpen)) {

        ImGui::Text("Caesar subject number");
        ImGui::PushItemWidth(ImGui::GetWindowWidth() * SLIDER_WIDTH * 0.68F);
        ImGui::InputInt("Subject##CaesarSubjectNumber", &_caesar_subject_number);
        ImGui::PopItemWidth();

        if (ImGui::Button("Quick Load")) {

            std::filesystem::path target_filename(globals::data_dir);
            target_filename /= std::string("caesar_fits");
            if (_mesh_type == MESH_MALE) {
                target_filename /= "male";
            } else {
                target_filename /= "female";
            }
            target_filename /= std::to_string(_caesar_subject_number) + "_skin.off";

            // target filename
            if (std::filesystem::exists(target_filename)) {
                load_target(target_filename);
            } else {
                std::cerr << "[Error] Target file " + std::string(target_filename) +" does not exist.\n";
            }
        }

        if (ImGui::Button("Load target")) {
            ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose target file", ".off,.obj", ".", 1, nullptr, ImGuiFileDialogFlags_Modal);
        }

        // display open file dialog
        if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey", ImGuiWindowFlags_NoCollapse, ImVec2(600.0F, 500.0F)))
        {
            // action if OK
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
                std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
                // action ---------------- load target -------------------
                load_target(filePathName);
            }

            // close
            ImGuiFileDialog::Instance()->Close();
        }

        ImGui::SameLine();
        if (ImGui::Button("Fit target") && _model != nullptr) {
            fit_target();
        }

        ImGui::Spacing();
        ImGui::Checkbox("Show Target", &_show_target_mesh);
        if (ImGui::Checkbox("Delta inference", &_inference_mode_delta)) {
            if (_model != nullptr) {
                _model->set_inference_mode(InferenceMode::NORMAL);
                if (_inference_mode_delta) {
                    _model->set_inference_mode(InferenceMode::FITTING_DELTA);
                }

                generate_meshes();
            }
        }

        if (ImGui::Checkbox("Post processing##PostProcEnabled", &_post_processing_enabled)) {
            if (_post_processing_enabled) {
                generate_meshes();
                do_processing();
            }
        }

        ImGui::Spacing();
        if (ImGui::Button("Run optimizers")) {
            if (_mesh != nullptr) {
                _mesh->optimize_meshes();
                update_meshes();
            }
        }
        ImGui::Spacing();
    }
}

//----------------------------------------------------------------------------------------------------------------------

auto TailorMeViewer::set_mesh(MeshType mesh_type) -> void
{
    if (_mesh_type != mesh_type) {
        _mesh_type = mesh_type;

        // free old mesh (if not nullptr)
        delete _mesh;

        switch (_mesh_type) {
        case MESH_UNDEFINED:
            _mesh = nullptr;
            break;
        case MESH_MALE:
            _mesh = new BodyMesh(MALE);
            break;
        case MESH_FEMALE:
            _mesh = new BodyMesh(FEMALE);
            break;
        }

        if (_mesh != nullptr) {
            // update postprocessing filter
            for (auto& filter : _post_processing_filters) {
                filter->update_meshes(
                    _mesh->submesh_vertex_count(SubMeshType::SUBMESH_SKELWRAP),
                    _mesh->submesh_vertex_count(SubMeshType::SUBMESH_SKIN)
                );
            }
        }

        if (_model != nullptr) {
            _model->set_mesh_type(_mesh_type);

            // use mesh data from model (if available)
            SurfaceMesh skel = _model->get_mean_skel();
            SurfaceMesh skin = _model->get_mean_skin();

            if (_mesh != nullptr && skel.n_vertices() > 0 && skin.n_vertices() > 0) {
                _mesh->set_skel(skel);
                _mesh->set_skin(skin);
            }

            // update weight buffer size
            int latent_dim_count = _model->latent_channels_sum();
            _latent_variables.resize(latent_dim_count);
            _latent_variables.setZero();

            // generate a new mesh from sliders
            init_head_stitcher();
            generate_meshes();
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------

void TailorMeViewer::set_model(ModelType model_type)
{
    if (_model_type != model_type) {
        _model_type = model_type;
        // free old model (null pointers get not deleted)
        delete _model;

        switch (_model_type) {
            case MODEL_UNDEFINED:
            _model = nullptr;
                break;
            case MODEL_SPIRAL_AE:
                _model = new SpiralNetAEModel();
                break;
        }

        // invoke new model generation by setting mesh
        const MeshType mesh_shown_ = _mesh_type;
        set_mesh(MeshType::MESH_UNDEFINED);
        set_mesh(mesh_shown_);
    }
}

// ---------------------------------------------------------------------------------------------------------------------

auto TailorMeViewer::rescale_weight_magnitude(float weight_magnitude) -> void
{
    // scale weights
    float factor = powf(WEIGHT_MAGNITUDE_BASE, _weight_magnitude) / powf(WEIGHT_MAGNITUDE_BASE, weight_magnitude);

    for (float & _latent_variable : _latent_variables) {
        _latent_variable *= factor;
    }

    _weight_magnitude = weight_magnitude;
}


//----------------------------------------------------------------------------------------------------------------------

auto TailorMeViewer::load_target(const std::string& filename) -> void
{
    std::cout << "Open target " << filename << '\n';
    _target_skin.open_file(filename);
    init_head_stitcher();
}

//----------------------------------------------------------------------------------------------------------------------

auto TailorMeViewer::fit_target() -> void
{
    if (_model == nullptr) {
        std::cerr << "[ERROR] Cannot fit target. No model loaded." << std::endl;
        return;
    }

    VectorXf target = _target_skin.get_mesh_points();
    if (static_cast<unsigned long> (target.size()) == _model->get_mean_skin().n_vertices() * 3) {
        _weight_magnitude = 0.0F; // reset weight magnitude
        _model->fit_target();
        _latent_variables = _model->get_latent_fit();

        // enable delta mode
        _model->set_inference_mode(InferenceMode::FITTING_DELTA);
        _inference_mode_delta = true;

        generate_meshes();
    } else {
        std::cerr << "Cannot fit! target_vertices=" << target.size() / 3 << " skin_vertices=" << _model->get_mean_skin().n_vertices() << '\n';
    }
}


// ---------------------------------------------------------------------------------------------------------------------

auto TailorMeViewer::perform_post_processing() -> void
{
    if (_mesh != nullptr) {
        // === run post-processing here
        auto* skel = _mesh->get_skel();
        auto* skin = _mesh->get_skin();
        if (skel != nullptr && skin != nullptr) {
            for (auto& filter : _post_processing_late_filters) {
                filter->postprocess(skel, skin);
            }

            _mesh->optimize_meshes();
            update_meshes();
        }

        _optimization_required = false;
    }
}

//----------------------------------------------------------------------------------------------------------------------

auto TailorMeViewer::init_head_stitcher() -> void
{
    if (_target_skin.get_mesh().n_vertices() != SKIN_VERTICES) {
        std::cout << "[Error] Head stitcher: Unexpected template format. Vertex count does not match.\n";
        return;
    }

    ArrayXf points = _target_skin.get_mesh_points();
    _model->set_target_skin(points);

    if (_mesh != nullptr && _mesh->get_skin() != nullptr && (_mesh_type == MESH_FEMALE || _mesh_type == MESH_MALE)
            && _target_skin.get_mesh().n_vertices() == _mesh->get_skin()->n_vertices())
    {
        std::cout << "Loaded new head \n";
        _mesh->update_layer_points(points, MeshLayer::LayerSkin);
        _mesh_stitcher.init(*_mesh->get_skin(), RESOURCE_DATA_DIR + "/head_inverse.sel");
    }
    generate_meshes();
}

//----------------------------------------------------------------------------------------------------------------------


//======================================================================================================================
