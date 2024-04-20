//======================================================================================================================
// Copyright (c) by Fabian Kemper 2022
//
// This work is licensed under a
// Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//
// You should have received a copy of the license along with this
// work. If not, see <http://creativecommons.org/licenses/by-nc-sa/4.0/>.
//
//======================================================================================================================

#include "BaseMesh.h"
#include "pmp/io/io.h"

//======================================================================================================================

using SurfaceMesh = pmp::SurfaceMesh;

//======================================================================================================================

// ---------------------------------------------------------------------------------------------------------------------

void BaseMesh::error_to_texture(SurfaceMesh& mesh, VectorXf& errors) {
    if (mesh.n_vertices() != static_cast<unsigned long> (errors.size())) {
        std::cout << "BaseMesh::error_to_texture: Error dimension mismatch.\n";
        errors = VectorXf { mesh.n_vertices() };
        errors.setRandom();
    }

    // sort values
    std::vector<pmp::Scalar> values;
    values.reserve(mesh.n_vertices());

    // search min & max
    for (auto e : errors) {
        values.push_back(static_cast<pmp::Scalar>(e));
    }
    std::sort(values.begin(), values.end());
    unsigned int n = values.size() - 1;

    // clamp upper / lower 5%
    unsigned int i = n / 20;
    auto min = values[i];
    auto max = values[n - 1 - i];

    auto tex = mesh.vertex_property<pmp::TexCoord>("v:tex");
    int index = 0;
    float tex_value;

    for (auto vertex : mesh.vertices()) {
        tex_value = (static_cast<float>(errors[index]) - min) / (max - min);
        // clamp
        if (tex_value < 0.0) {
            tex_value = 0.0;
        }
        if (tex_value > 1.0) {
            tex_value = 1.0;
        }
        // complete space
        tex[vertex] = pmp::TexCoord(tex_value, 0.0);
        // only red area
        //tex[v] = TexCoord(0.5f + 0.5f * (static_cast<float>(errors[index]) - min) / (max - min), 0.0);
        index++;
    }
}

// ---------------------------------------------------------------------------------------------------------------------

auto BaseMesh::submesh_vertex_count(SubMeshType submesh_type) -> long
{
    (void) submesh_type;
    return 0;
}

// ---------------------------------------------------------------------------------------------------------------------

void BaseMesh::save_meshes(const std::string& bone_filename, const std::string& skel_filename, const std::string& skin_filename)
{
    pmp::IOFlags flags {};
    flags.use_vertex_texcoords = true;

    pmp::write(*get_bone(), bone_filename, flags);
    pmp::write(*get_skel(), skel_filename, flags);
    pmp::write(*get_skin(), skin_filename, flags);
}

// ---------------------------------------------------------------------------------------------------------------------

auto BaseMesh::get_bone() -> pmp::SurfaceMesh*
{
    std::cerr << "Get skel not overwritten.\n";
    return {};
}

// ---------------------------------------------------------------------------------------------------------------------

auto BaseMesh::get_skel() -> pmp::SurfaceMesh*
{
    std::cerr << "Get skel not overwritten.\n";
    return {};
}

// ---------------------------------------------------------------------------------------------------------------------

auto BaseMesh::get_skin() -> pmp::SurfaceMesh*
{
    std::cerr << "Get skin not overwritten.\n";
    return {};
}

// ---------------------------------------------------------------------------------------------------------------------

auto BaseMesh::set_skel(SurfaceMesh& mesh) -> void
{
    (void) mesh;
    std::cerr << "Set skel not overwritten.\n";
}

// ---------------------------------------------------------------------------------------------------------------------

auto BaseMesh::set_skin(SurfaceMesh& mesh) -> void
{
    (void) mesh;
    std::cerr << "Set skin not overwritten.\n";
}

// ---------------------------------------------------------------------------------------------------------------------

auto BaseMesh::optimize_meshes() -> void
{

}

// ---------------------------------------------------------------------------------------------------------------------

auto BaseMesh::update_layer_points(ArrayXf& point_data, MeshLayer layer) -> void
{
    (void) point_data;
    (void) layer;
}

// ---------------------------------------------------------------------------------------------------------------------

//======================================================================================================================
