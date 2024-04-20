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

#ifndef TAILORME_VIEWER_H
#define TAILORME_VIEWER_H

// ---------------------------------------------------------------------------------------------------------------------
// includes
#include <pmp/visualization/trackball_viewer.h>

#include "algorithms/MeshStitching.h"

#include "models/BaseModel.h"

#include "meshes/BodyMesh.h"
#include "meshes/TargetSkinMesh.h"

#include "mesh_massage/post_processing_base.h"

// ---------------------------------------------------------------------------------------------------------------------
// definitions

#define WEIGHT_MAGNITUDE_BASE 10.0F


// ---------------------------------------------------------------------------------------------------------------------

class TailorMeViewer : public pmp::TrackballViewer {
public:
    //! constructor
  TailorMeViewer(const char* title, int width, int height, bool show_gui = true);

    //! destructor
    ~TailorMeViewer() override = default;

    //! draw the scene
    void draw(const std::string& _draw_mode) override;

    //! handle ImGUI interface
    void process_imgui() override;

    void do_processing() override;

protected:
    // --- meshes ---
    // current mesh pointer (used for draw and inference)
    BaseMesh* _mesh = nullptr;

    // prediction model
    BaseModel* _model = nullptr;

    // target skin (for fitting)
    TargetSkinMesh _target_skin = TargetSkinMesh();

    Mesh_stitcher _mesh_stitcher;

    // input variables
    ArrayXf _latent_variables{};

    // scale of latent variables - exponential
    float _weight_magnitude = 0.0;

    bool _optimization_required = false;
    bool _optimization_suitable = true;
    int  _frames_without_user_interaction = 0;

    bool _show_unnamed_sliders = true;

    //! transparency value for skin rendering
    float _opacity_bone = 1.0F;
    float _opacity_skel = 1.0F;
    float _opacity_skin = 1.0F;

    // For fast loading caesar mesh based on subject number
    int _caesar_subject_number = 4000;

    //! show target mesh
    bool _show_target_mesh = false;
    bool _inference_mode_delta = false;

    //! post processing
    bool _post_processing_enabled = true;

    //! shown mesh
    MeshType _mesh_type = MeshType::MESH_UNDEFINED;
    ModelType _model_type = ModelType::MODEL_UNDEFINED;

    //! post processing (direct and delayed)
    std::vector<std::unique_ptr<PostProcessingBase>> _post_processing_filters {};
    std::vector<std::unique_ptr<PostProcessingBase>> _post_processing_late_filters {};

    // update each of these components
    auto set_mesh(MeshType mesh_type) -> void;
    // load model
    auto set_model(ModelType model_type) -> void;
    // set parameter range
    auto rescale_weight_magnitude(float weight_magnitude) -> void;

    // imgui for debug menu
    auto process_imgui_model() -> void;
    auto process_imgui_weights() -> void;
    auto process_imgui_fitting() -> void;

    // create initial bouding box and view angle
    void update_bb();
    // -- compute face and vertex normals, update face indices
    void update_meshes();

    // -- recalculate meshes
    void generate_meshes();

    // -- load target
    auto load_target(const std::string& filename) -> void;
    auto fit_target() -> void;

    auto perform_post_processing() -> void;
    auto init_head_stitcher() -> void;
};


#endif //TAILORME_VIEWER_H
