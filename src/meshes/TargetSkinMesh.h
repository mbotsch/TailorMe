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

#ifndef TAILORME_VIEWER_TARGETSKINMESH_H
#define TAILORME_VIEWER_TARGETSKINMESH_H

#include "GlobTypes.h"

#include <pmp/surface_mesh.h>
#include <pmp/visualization/renderer.h>

#include "meshes/BodyMesh.h"


class TargetSkinMesh {
  protected:
    BodyType _body_type = MALE;
    pmp::SurfaceMesh* _mesh = new pmp::SurfaceMesh(); // placeholder
    pmp::Renderer* _renderer = nullptr;

    // get foot point
    auto _get_foot_point() -> pmp::Point;

  public:
    auto draw(const pmp::mat4 &projection_matrix,
              const pmp::mat4 &modelview_matrix,
              const std::string &draw_mode) -> void;

    auto get_mesh() -> pmp::SurfaceMesh;
    auto get_mesh_points() -> VectorXf;

    auto open_file(const std::string& filename) -> void;
    auto set_body_type(BodyType body_type) -> void;

    auto update_meshes() -> void;
};

#endif // TAILORME_VIEWER_TARGETSKINMESH_H
