//======================================================================================================================
// Copyright (c) the Authors 2024
//
// This work is licensed under a
// Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//
// You should have received a copy of the license along with this
// work. If not, see <http://creativecommons.org/licenses/by-nc-sa/4.0/>.
//
//======================================================================================================================

#include "TargetSkinMesh.h"
#include "Constants.h"
#include "pmp/io/io.h"

// =====================================================================================================================

// ---------------------------------------------------------------------------------------------------------------------

void TargetSkinMesh::draw(const pmp::mat4& projection_matrix, const pmp::mat4& modelview_matrix,
                          const std::string& draw_mode)
{
    if (_renderer != nullptr) {
        pmp::Point foot_point = (-1) * _get_foot_point();//+ pmp::Point{1.35, -.60, .25};

        pmp::mat4 mv_body = modelview_matrix * pmp::translation_matrix(foot_point);

        _renderer->draw(projection_matrix, mv_body, draw_mode);
    }
}

// ---------------------------------------------------------------------------------------------------------------------

void TargetSkinMesh::update_meshes()
{
    if (_renderer != nullptr) {
        _renderer->update_opengl_buffers();
    }
}

// ---------------------------------------------------------------------------------------------------------------------

auto TargetSkinMesh::get_mesh() -> pmp::SurfaceMesh
{
    return *_mesh;
}

// ---------------------------------------------------------------------------------------------------------------------

auto TargetSkinMesh::get_mesh_points() -> VectorXf
{
    VectorXf result {};
    if (_mesh != nullptr) {
        result.resize(_mesh->n_vertices() * 3);
        long offset = 0;

        for (auto vertex : _mesh->vertices()) {
            auto position = _mesh->position(vertex);
            result[offset + 0] = position[0];
            result[offset + 1] = position[1];
            result[offset + 2] = position[2];
//            std::memcpy(result.data() + offset * sizeof(double), position.data(), 3 * sizeof(double));
            offset += 3;
        }
    }

    return result;
}

// ---------------------------------------------------------------------------------------------------------------------

auto TargetSkinMesh::open_file(const std::string& filename) -> void
{
    if (_mesh != nullptr) {
        delete _mesh;
        delete _renderer;
    }

    _mesh = new pmp::SurfaceMesh();
    pmp::read(*_mesh, filename);
    _renderer = new pmp::Renderer(*_mesh);

    // render settings
    auto skin_matcap_filename = std::filesystem::path(RESOURCE_DATA_DIR) / MATCAP_SKIN;

    if (std::filesystem::exists(skin_matcap_filename)) {
        _renderer->load_matcap(skin_matcap_filename.string().c_str());
    }

    // call draw_normal - but no virtual methods in constructor
    _renderer->set_shininess(50.0);
    _renderer->set_specular(0.05);
    _renderer->set_front_color(pmp::vec3(0.9, 0.75, 0.68));

    update_meshes();
}

// ---------------------------------------------------------------------------------------------------------------------

auto TargetSkinMesh::set_body_type(BodyType body_type) -> void
{
    _body_type = body_type;
}

// ---------------------------------------------------------------------------------------------------------------------


auto TargetSkinMesh::_get_foot_point() -> pmp::Point
{
    if (_mesh != nullptr && _mesh->n_vertices() == 23752) {
        return (_mesh->position(pmp::Vertex(5283)) + _mesh->position(pmp::Vertex(15901))) / 2.0F;
    }
    return pmp::Point(0.0F, 0.0F, 0.0F);
}


// ---------------------------------------------------------------------------------------------------------------------

