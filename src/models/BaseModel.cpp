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

#include "BaseModel.h"

#include <iostream>

// =====================================================================================================================

using SurfaceMesh = pmp::SurfaceMesh;

// =====================================================================================================================

// ---------------------------------------------------------------------------------------------------------------------

auto BaseModel::model_type() -> ModelType
{
    return _model_type;
}

// ---------------------------------------------------------------------------------------------------------------------

auto BaseModel::extract_to_buffer(const libz::ZipArchive& archive, const std::string& entry_name, std::stringstream& buffer) -> void
{
    libz::ZipEntry entry = archive.getEntry(entry_name);

    // does entry exists?
    if (entry.isNull()) {
        std::ostringstream strm;
        strm << "File " << entry_name << " not found in zip-file.";
        throw std::runtime_error(strm.str());
    }

    // clear buffer
    buffer.str(std::string());
    // read to buffer
    entry.readContent(buffer, libz::ZipArchive::State::Original);
    // set reading head to start of stream (for later reading)
    buffer.seekp(0, std::ios_base::beg);
}

// ---------------------------------------------------------------------------------------------------------------------

auto BaseModel::inference(const ArrayXf &weights) -> ArrayXf
{
    _marked_for_inference = false;
    return Eigen::ArrayXf { weights };
}

// ---------------------------------------------------------------------------------------------------------------------

auto BaseModel::inference_available() const -> bool
{
    std::cerr << "Overwrite inference_available in your model.\n";
    return false;
}

// ---------------------------------------------------------------------------------------------------------------------

auto BaseModel::latent_dimensions() -> int
{
    return 1;
}

// ---------------------------------------------------------------------------------------------------------------------

auto BaseModel::latent_dimension_name(int dimension) -> std::string
{
    (void) dimension;
    return {};
}

// ---------------------------------------------------------------------------------------------------------------------

auto BaseModel::latent_channels_sum() -> int
{
    int result = 0;
    for (int index = 0; index < latent_dimensions(); ++index) {
        result += latent_channels(index);
    }
    return result;
}

// ---------------------------------------------------------------------------------------------------------------------

auto BaseModel::latent_channels(int dimension) -> int
{
    // suppress warning
    (void) dimension;

    return 0;
}

// ---------------------------------------------------------------------------------------------------------------------

auto BaseModel::latent_channel_name(int dimension, int channel) -> std::string
{
    (void) dimension; (void) channel;
    return {};
}

// ---------------------------------------------------------------------------------------------------------------------

auto BaseModel::set_mesh_type(MeshType mesh_type) -> void
{
    _mesh_type = mesh_type;
}

// ---------------------------------------------------------------------------------------------------------------------

auto BaseModel::get_mean_skel() -> SurfaceMesh
{
    return {};
}

// ---------------------------------------------------------------------------------------------------------------------

auto BaseModel::get_mean_skin() -> SurfaceMesh
{
    return {};
}

// ---------------------------------------------------------------------------------------------------------------------

auto BaseModel::set_target_skin(ArrayXf& target_skin) -> void {
    (void) target_skin;
}

// ---------------------------------------------------------------------------------------------------------------------

auto BaseModel::fit_target() -> void
{

}

// ---------------------------------------------------------------------------------------------------------------------

auto BaseModel::get_latent_fit() -> ArrayXf
{
    return ArrayXf{};
}

// ---------------------------------------------------------------------------------------------------------------------

auto BaseModel::set_inference_mode(InferenceMode mode) -> void
{
    _inference_mode = mode;
}

// ---------------------------------------------------------------------------------------------------------------------

// =====================================================================================================================
