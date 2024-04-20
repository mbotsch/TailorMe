//
// Created by fabian on 12/09/22.
//

#ifndef MUTLILINEAR_BODYMESH_H
#define MUTLILINEAR_BODYMESH_H

#include <pmp/visualization/renderer.h>

#include "BaseMesh.h"

#include "algorithms/RBF_warp.h"

enum BodyType {
    MALE,
    FEMALE,
};

class BodyMesh : public BaseMesh {
protected:
    pmp::SurfaceMesh _skel_wrap{};
    pmp::SurfaceMesh _skin{};

    pmp::SurfaceMesh _template_skel{};
    pmp::SurfaceMesh _template_bones{};
    pmp::SurfaceMesh _bones{};

    pmp::Renderer _skel_renderer;
    pmp::Renderer _skin_renderer;
    pmp::Renderer _bone_renderer;

    RBF_data _rbf_data{};

    BodyType _gender;

    auto _get_foot_point() -> pmp::Point;

public:
    explicit BodyMesh(BodyType gender);

    void draw(const pmp::mat4 &projection_matrix,
              const pmp::mat4 &modelview_matrix,
              const std::string &draw_mode) override;

    void update_meshes() override;

    // update mesh point data
    void update_mesh_points(VectorXf &point_data) override;
    auto update_layer_points(ArrayXf &point_data, MeshLayer layer) -> void override;

    // get vertex count for specified submesh
    auto submesh_vertex_count(SubMeshType submesh_type) -> long override;

    // get and set openGL meshes
    auto get_bone() -> pmp::SurfaceMesh* override;
    auto get_skel() -> pmp::SurfaceMesh* override;
    auto get_skin() -> pmp::SurfaceMesh* override;

    auto set_skel(pmp::SurfaceMesh& mesh) -> void override;
    auto set_skin(pmp::SurfaceMesh& mesh) -> void override;

    // optimize meshes
    auto optimize_meshes() -> void override;
};


#endif //MUTLILINEAR_BODYMESH_H
