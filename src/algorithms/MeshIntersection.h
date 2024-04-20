#ifndef TAILORME_VIEWER_MESHINTERSECTION_H
#define TAILORME_VIEWER_MESHINTERSECTION_H

#include <pmp/surface_mesh.h>

class MeshIntersection {
public:
//    auto static mesh_to_verts_faces(const pmp::SurfaceMesh* mesh, Eigen::MatrixXi& vertices, Eigen::MatrixXi& faces) -> void;
    auto static mesh_intersection(const pmp::SurfaceMesh* mesh_a, const pmp::SurfaceMesh* mesh_b) -> bool;

    // tracked version, adds a edge property for intersections e:feature
    auto static mesh_intersection_tracked(pmp::SurfaceMesh* mesh_a, pmp::SurfaceMesh* mesh_b) -> int;
};


#endif //TAILORME_VIEWER_MESHINTERSECTION_H
