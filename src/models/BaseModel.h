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

#ifndef TAILORME_VIEWER_BASEMODEL_H
#define TAILORME_VIEWER_BASEMODEL_H

#include <string>

#include <Eigen/Eigen>
#include "libzippp.h"

#include "meshes/BaseMesh.h"

// namespaces
//using namespace Eigen;
namespace libz = libzippp;

// === Types for models
enum ModelType {
    MODEL_UNDEFINED,
    // barlow twins, autoencoder
    MODEL_SPIRAL_AE,
};

// === Which mode should be used for model inference?
enum InferenceMode {
    // normal inference = latent parameters in, and direct output
    NORMAL,
    // fitting mode
    // use input as offset for fittet latent values
    // let X be the set of meshes
    // let Z be the space of latent variables
    // let f: Z -> X be the data generator
    // let x be the ground truth target
    // let z be the latent variables for target skin x (ground truth)
    // let z' be the user input values for latent variables
    // let x' be the result shown to the user
    // return x' = x + (f(z) - f(z'))
    FITTING_DELTA
};
// TODO: Implement inference mode


// === Base Model class
class BaseModel {
  protected:
    MeshType _mesh_type = MeshType::MESH_UNDEFINED;
    ModelType _model_type = ModelType::MODEL_UNDEFINED;
    // make a new inference on next render
    bool _marked_for_inference = false;

    // inference mode
    InferenceMode _inference_mode = NORMAL;

    // helper, extract from zip to stringstream buffer
    auto static extract_to_buffer(const libz::ZipArchive& archive, const std::string& entry_name, std::stringstream& buffer) -> void;

  public:
    BaseModel() = default;
    virtual ~BaseModel() = default;

    // type switch
    virtual auto model_type() -> ModelType;

    // marked for inference?
    [[nodiscard]]
    auto get_marked_for_inference() const -> bool { return _marked_for_inference; }

    // inference a model by weights
    virtual auto inference(const ArrayXf& weights) -> ArrayXf;

    // inference available (module loaded)
    [[nodiscard]]
    virtual auto inference_available() const -> bool;

    // number of latent features
    virtual auto latent_dimensions() -> int;
    virtual auto latent_dimension_name(int dimension) -> std::string;
    virtual auto latent_channels_sum() -> int;
    virtual auto latent_channels(int dimension) -> int;
    virtual auto latent_channel_name(int dimension, int channel) -> std::string;

    // set mesh
    virtual auto set_mesh_type(MeshType mesh_type) -> void;

    // get mesh from trained model
    virtual auto get_mean_skel() -> pmp::SurfaceMesh;
    virtual auto get_mean_skin() -> pmp::SurfaceMesh;

    // set a fitting skin target (copy values into model)
    // setting x
    virtual auto set_target_skin(ArrayXf& target_skin) -> void;
    // perform fitting (this is subject to change)
    // this should set the "best fit" skin internally for delta changes
    // setting z
    virtual auto fit_target() -> void;
    // get fitted values for latent variables
    virtual auto get_latent_fit() -> ArrayXf;
    // set prediction mode normal vs. fitting delta
    auto set_inference_mode(InferenceMode mode) -> void;

};


#endif //TAILORME_VIEWER_BASEMODEL_H
