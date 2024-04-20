//
// Created by fabian on 12/09/22.
//
// =====================================================================================================================

#include "BodyMesh.h"
#include "pmp/algorithms/normals.h"
#include "pmp/io/read_off.h"
#include "pmp/io/read_obj.h"
#include "pmp/stop_watch.h"

#include "Constants.h"
#include "utils/io/io_selection.h"
#include "utils/io/io_vertexweighting.h"

#include "algorithms/LayerCollisionResolve.h"

#include <filesystem>

// =====================================================================================================================

using SurfaceMesh = pmp::SurfaceMesh;

// =====================================================================================================================

BodyMesh::BodyMesh(BodyType gender)
    : _skel_renderer(_skel_wrap), _skin_renderer(_skin), _bone_renderer(_bones), _gender(gender)
{
    try {
        if (_gender == MALE) {
            read_off(_skel_wrap, std::filesystem::path(RESOURCE_DATA_DIR) / "mean_male_skel.off");
            read_off(_skin, std::filesystem::path(RESOURCE_DATA_DIR) / "mean_male_skin.off");
            read_obj(_template_bones, std::filesystem::path(RESOURCE_DATA_DIR) / "template_male_bones.obj");
            read_off(_template_skel, std::filesystem::path(RESOURCE_DATA_DIR) / "template_male_skel.off");
        } else {
            read_off(_skel_wrap, std::filesystem::path(RESOURCE_DATA_DIR) / "mean_female_skel.off");
            read_off(_skin, std::filesystem::path(RESOURCE_DATA_DIR) / "mean_female_skin.off");
            read_obj(_template_bones, std::filesystem::path(RESOURCE_DATA_DIR) / "template_female_bones.obj");
            read_off(_template_skel, std::filesystem::path(RESOURCE_DATA_DIR) / "template_female_skel.off");
        }
    } catch (pmp::IOException& ioException) {
        std::cerr << ioException.what() << '\n';
    }

    if (_skel_wrap.n_vertices() != SKEL_PARTIAL_WRAP_VERTICES) {
        std::cerr << "[ERROR] Skeleton wrap vertex count do not match expected value.\n";
    }
    if (_skin.n_vertices() != SKIN_VERTICES) {
        std::cerr << "[ERROR] Skin vertex count do not match expected value.\n";
    }
    if (_template_bones.n_vertices() != SKEL_HIRES_VERTICES) {
        std::cerr << "[ERROR] High-Resolution skeleton vertex count do not match expected value.\n";
    }
    if (_template_skel.n_vertices() != SKEL_FULL_WRAP_VERTICES) {
        std::cerr << "[ERROR] Complete skeleton wrap vertex count do not match expected value.\n";
    }


    // locked selection
    std::vector<int> locked_selection;
    if (!read_selection(std::filesystem::path(RESOURCE_DATA_DIR) / "bo_head_hands_toes.sel", locked_selection))
    {
        std::cerr << "[ERROR] Cannot load locked vertex selection from " << RESOURCE_DATA_DIR << "/bo_head_hands_toes.sel" << std::endl;
    }

    // intersection ignore list (mouth model is intersecting by default)
    //    if (_skin.has_vertex_property("v:intersection_ignore")) {
    //        auto v_intersection_ignore = _skin.get_vertex_property<bool>("v:intersection_ignore");
    //        _skin.remove_vertex_property(v_intersection_ignore);
    //    }
    auto v_intersection_ignore = _skin.vertex_property<bool>("v:intersection_ignore", false);

    // read mouth vertices
    std::vector<int> mouth_selection;
    if (!read_selection(std::filesystem::path(RESOURCE_DATA_DIR) / "mouth.sel", mouth_selection)) {
        std::cerr << "[ERROR] Cannot load mouth vertex selection from " << RESOURCE_DATA_DIR << "/mouth.sel" << std::endl;
    }

    // mark vertices which should not be marked intersecting
    for (auto idx : mouth_selection) {
        v_intersection_ignore[pmp::Vertex(idx)] = true;
    }

    auto locked_prop = _skin.vertex_property<bool>("v:collision_resolve_locked", false);
    for (int idx : locked_selection)
    {
        locked_prop[pmp::Vertex(idx)] = true;
    }

    // Cutoff parts of the wraps (head, hands, toes) are not handled well in the RBF warp.
    // For this reason, we slightly shrink the wrap in these places by the specified
    // vertexweighting.
    load_vertexweighting(std::filesystem::path(RESOURCE_DATA_DIR) / "wrap_shrinking.vw", _skin, "v:rbf_warp_shrink_wrap");

    _bones = _template_bones;

    // MatCap: Skeleton
    auto skel_matcap_filename= std::filesystem::path(RESOURCE_DATA_DIR) / MATCAP_BONE;

    if (std::filesystem::exists(skel_matcap_filename)) {
        _skel_renderer.load_matcap(skel_matcap_filename.string().c_str());
        _bone_renderer.load_matcap(skel_matcap_filename.string().c_str());
    }

    // MatCap: Skin
    auto skin_matcap_filename= std::filesystem::path(RESOURCE_DATA_DIR) / MATCAP_SKIN;

    if (std::filesystem::exists(skin_matcap_filename)) {
        _skin_renderer.load_matcap(skin_matcap_filename.string().c_str());
    }

    // call draw_normal - but no virtual methods in constructor
    _bone_renderer.set_shininess(5.00);
    _bone_renderer.set_specular(0.05);
    _bone_renderer.set_front_color(pmp::vec3(0.945, 0.859, 0.694));

    _skel_renderer.set_shininess(5.00);
    _skel_renderer.set_specular(0.05);
    _skel_renderer.set_front_color(pmp::vec3(0.945, 0.859, 0.694));

    _skin_renderer.set_shininess(50.0);
    _skin_renderer.set_specular(0.25);
    _skin_renderer.set_front_color(pmp::vec3(0.698, 0.631, 0.808));
}

// ---------------------------------------------------------------------------------------------------------------------

auto BodyMesh::get_bone() -> SurfaceMesh* {
    return &_bones;
}

// ---------------------------------------------------------------------------------------------------------------------

auto BodyMesh::_get_foot_point() -> pmp::Point {
    if (_skin.n_vertices() == SKIN_VERTICES) {
        return (_skin.position(pmp::Vertex(5283)) + _skin.position(pmp::Vertex(15901))) / 2.0F;
    }
    return pmp::Point(0.0F, 0.0F, 0.0F);
}

// ---------------------------------------------------------------------------------------------------------------------

auto BodyMesh::get_skel() -> SurfaceMesh* {
    return &_skel_wrap;
}

// ---------------------------------------------------------------------------------------------------------------------

auto BodyMesh::get_skin() -> SurfaceMesh* {
    return &_skin;
}

// ---------------------------------------------------------------------------------------------------------------------

auto BodyMesh::set_skel(SurfaceMesh& mesh) -> void
{ _skel_wrap = SurfaceMesh { mesh };
}

// ---------------------------------------------------------------------------------------------------------------------

auto BodyMesh::set_skin(SurfaceMesh& mesh) -> void
{
    if (!_skin.is_empty())
    {
        auto locked_prop_new = mesh.vertex_property<bool>("v:collision_resolve_locked", false);
        auto locked_prop_old = _skin.vertex_property<bool>("v:collision_resolve_locked");

        auto intersection_ignore_prop_new = mesh.vertex_property<bool>("v:intersection_ignore", false);
        auto intersection_ignore_prop_old = _skin.vertex_property<bool>("v:intersection_ignore");

        auto shrink_prop_new = mesh.vertex_property<float>("v:rbf_warp_shrink_wrap", 0.0f);
        auto shrink_prop_old = _skin.vertex_property<float>("v:rbf_warp_shrink_wrap");

        for (auto v : mesh.vertices())
        {
            locked_prop_new[v] = locked_prop_old[v];
            intersection_ignore_prop_new[v] = intersection_ignore_prop_old[v];
            shrink_prop_new[v] = shrink_prop_old[v];
        }
    }

    _skin = SurfaceMesh { mesh };
}

// ---------------------------------------------------------------------------------------------------------------------

void BodyMesh::draw(const pmp::mat4& projection_matrix, const pmp::mat4& modelview_matrix, const std::string& draw_mode) {
    pmp::Point foot_point = (-1.0F) * _get_foot_point();

    pmp::mat4 mv_body = modelview_matrix
                        * pmp::translation_matrix(foot_point);

    // _template_skel_renderer.draw(projection_matrix, mv_body, draw_mode);
    glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);

    if (_alpha_bone > 0.0) {
        _bone_renderer.set_alpha(_alpha_bone);
        _bone_renderer.draw(projection_matrix, mv_body, draw_mode);
    }

    if (_alpha_skel > 0.0) {
        _skel_renderer.set_alpha(_alpha_skel);
        _skel_renderer.draw(projection_matrix, mv_body, draw_mode);
    }

    if (_alpha_skin > 0.0) {
        _skin_renderer.set_alpha(_alpha_skin);
        _skin_renderer.draw(projection_matrix, mv_body, draw_mode);
    }
    glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
}

// ---------------------------------------------------------------------------------------------------------------------

void BodyMesh::update_meshes() {
    _bone_renderer.update_opengl_buffers();
    _skel_renderer.update_opengl_buffers();
    _skin_renderer.update_opengl_buffers();
}

// ---------------------------------------------------------------------------------------------------------------------

auto BodyMesh::submesh_vertex_count(SubMeshType submesh_type) -> long {
    if (submesh_type == SubMeshType::SUBMESH_SKEL) {
        return static_cast<long>(_bones.n_vertices());
    }
    if (submesh_type == SubMeshType::SUBMESH_SKELWRAP) {
        return static_cast<long>(_skel_wrap.n_vertices());
    }
    if (submesh_type == SubMeshType::SUBMESH_SKIN) {
        return static_cast<long>(_skin.n_vertices());
    }

    return 0;
}

// ---------------------------------------------------------------------------------------------------------------------

void BodyMesh::update_mesh_points(VectorXf &point_data) {
    assert(point_data.size() == (_skel_wrap.n_vertices() + _skin.n_vertices()) * 3);
    mesh_points_ = point_data;

    int offset = 0;
    for (pmp::Vertex vertex : _skel_wrap.vertices()) {
        _skel_wrap.position(vertex) = pmp::Point(
            static_cast<float> (point_data[offset+0]),
            static_cast<float> (point_data[offset+1]),
            static_cast<float> (point_data[offset+2])
        );
        offset += 3;
    }

    for (pmp::Vertex vertex : _skin.vertices()) {
        _skin.position(vertex) = pmp::Point {
            static_cast<float> (point_data[offset+0]),
            static_cast<float> (point_data[offset+1]),
            static_cast<float> (point_data[offset+2])
        };
        offset += 3;
    }
    update_meshes();
}

// ---------------------------------------------------------------------------------------------------------------------

auto BodyMesh::optimize_meshes() -> void
{
//    std::cout << "Run mesh optimizer\n";
    BaseMesh::optimize_meshes();

    if (_rbf_data.num_centers == 0)
    {
        // Need to initialize and prefactorize RBF warp
        std::cout << "Prefatorizing rbf warp\n";
        init_rbf_warp_prioritize_head(_template_skel, 4800, _rbf_data);
    }

    // Reset bone mesh to template
    _bones = _template_bones;

    // Get full skel mesh (setting cut off vertices to skin)
    pmp::SurfaceMesh full_skel_wrap = _skin;
    for (auto v : full_skel_wrap.vertices())
    {
        int mapped_idx = _rbf_data.mapping_full_to_cutoff[v.idx()];
        if (mapped_idx != -1)
        {
            auto mapped_v = pmp::Vertex(mapped_idx);
            full_skel_wrap.position(v) = _skel_wrap.position(mapped_v);
        }
    }
    // Push the wrap a few millimeters inside at head, hands and feet
    pmp::vertex_normals(full_skel_wrap);
    auto shrink_prop = _skin.get_vertex_property<float>("v:rbf_warp_shrink_wrap");
    auto vnormal = full_skel_wrap.vertex_property<pmp::Normal>("v:normal");

    for (auto v : full_skel_wrap.vertices())
    {
        full_skel_wrap.position(v) -= shrink_prop[v] * vnormal[v];
    }

    auto locked_prop = _skin.get_vertex_property<bool>("v:collision_resolve_locked");
    resolve_layer_intersections_by_bottom_layer(_skin, full_skel_wrap, locked_prop);

    // Remap from full to reduced skel
    for (auto v : full_skel_wrap.vertices())
    {
        int mapped_idx = _rbf_data.mapping_full_to_cutoff[v.idx()];
        if (mapped_idx != -1)
        {
            auto mapped_v = pmp::Vertex(mapped_idx);
            _skel_wrap.position(mapped_v) = full_skel_wrap.position(v);
        }
    }

    // Warp bones into skel wrap
    apply_rbf_warp(full_skel_wrap, _bones, _rbf_data);

    _skel_renderer.update_opengl_buffers();
    _bone_renderer.update_opengl_buffers();
}

// ---------------------------------------------------------------------------------------------------------------------

auto BodyMesh::update_layer_points(ArrayXf& point_data, MeshLayer layer) -> void
{
    if (layer == LayerBone) {
        std::cout << "no idea for bones\n";
    }

    if (layer == LayerSkel) {
        std::cout << "no idea for skel\n";
    }

    if (layer == LayerSkin) {
        if (point_data.size() == static_cast<long> (_skin.n_vertices() * 3)) {
            std::cout << "[DEBUG] Updated skin points\n";
            std::memcpy(_skin.position(pmp::Vertex(0)).data(), point_data.data(), point_data.size() * sizeof (float));
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------

// =====================================================================================================================
