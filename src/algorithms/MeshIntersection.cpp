// =====================================================================================================================

#include "MeshIntersection.h"
#include "algorithms/TriTriIntersect.h"

#include <iostream>

// =====================================================================================================================


/*
auto MeshIntersection::mesh_to_verts_faces(
    const pmp::SurfaceMesh *mesh,
    Eigen::MatrixXi &vertices,
    Eigen::MatrixXi &faces)
-> void
{

}
*/

// ---------------------------------------------------------------------------------------------------------------------

auto MeshIntersection::mesh_intersection(const pmp::SurfaceMesh *mesh_a, const pmp::SurfaceMesh *mesh_b)
-> bool
{
    // check only for debugging
    if (!mesh_a->is_triangle_mesh() || !mesh_b->is_triangle_mesh()) {
        throw std::runtime_error("Mesh intersection is only implemented for triangle meshes.");
    }

    // save vertex positions
    // there might be a possibility to access vertex positions directly
    // TODO: Ask Timo
//    std::vector<pmp::Point> mesh_a_points { mesh_a->n_vertices() };
//    std::vector<pmp::Point> mesh_b_points { mesh_b->n_vertices() };

    // helper type definition
    using PointT = Eigen::RowVector3f;
    using TriangleT = std::array<PointT, 3>;

    size_t v_index; // vertex index
    size_t f_index; // face index
    std::vector<TriangleT> faces_a{mesh_a->n_faces()};
    std::vector<TriangleT> faces_b{mesh_b->n_faces()};

    // todo: externalize to function

    // convert to array structure for faster access
    f_index = 0;
    for (auto face: mesh_a->faces()) {
        v_index = 0;
        TriangleT triangle{PointT{}, PointT{}, PointT{}};
        for (auto vertex: mesh_a->vertices(face)) {
            pmp::Point point = mesh_a->position(vertex);
            triangle[v_index++] = PointT{point[0], point[1], point[2]};
        }
        faces_a[f_index++] = triangle;
    }

    f_index = 0;
    for (auto face: mesh_b->faces()) {
        v_index = 0;
        TriangleT triangle{PointT{}, PointT{}, PointT{}};
        for (auto vertex: mesh_b->vertices(face)) {
            pmp::Point point = mesh_b->position(vertex);
            triangle[v_index++] = PointT{point[0], point[1], point[2]};
        }
        faces_b[f_index++] = triangle;
    }

    // out variables for intersection
    PointT int_start{};
    PointT int_end{};
    bool coplanar;

    // check intersection for every triangle
    for (size_t index_a = 0; index_a < mesh_a->n_faces(); ++index_a) {
        TriangleT& tri_a = faces_a[index_a];

        for (size_t index_b = 0; index_b < mesh_b->n_faces(); ++index_b) {
            TriangleT& tri_b = faces_b[index_b];
            //std::cout << "triA" << tri_a[0] << " tri_b" << tri_b[0] << std::endl;

            if (igl::tri_tri_intersection_test_3d(
                tri_a[0], tri_a[1], tri_a[2],
                tri_b[0], tri_b[1], tri_b[2],
                coplanar, int_start, int_end)
            ) {
                // one intersection is enough
                std::cout << "Intersection Face_A=" << index_a << " and Face_B=" << index_b << std::endl;
                return true;
            }
        }
    }

    // no intersection
    return false;
}

auto mark_intersection(pmp::SurfaceMesh* mesh, pmp::EdgeProperty<bool>& e_feature_prop, pmp::Face face) -> void
{
    for (auto he : mesh->halfedges(face)) {
        e_feature_prop[mesh->edge(he)] = true;
    }
}

// ---------------------------------------------------------------------------------------------------------------------

auto MeshIntersection::mesh_intersection_tracked(pmp::SurfaceMesh *mesh_a, pmp::SurfaceMesh *mesh_b)
    -> int
{
    // check only for debugging
    if (!mesh_a->is_triangle_mesh() || !mesh_b->is_triangle_mesh()) {
        throw std::runtime_error("Mesh intersection is only implemented for triangle meshes.");
    }

    int intersections = 0;

    // save vertex positions
    // there might be a possibility to access vertex positions directly
    // TODO: Ask Timo
    //    std::vector<pmp::Point> mesh_a_points { mesh_a->n_vertices() };
    //    std::vector<pmp::Point> mesh_b_points { mesh_b->n_vertices() };

    // helper type definition
    using PointT = Eigen::RowVector3f;
    using TriangleT = std::array<PointT, 3>;
    size_t v_index; // vertex index

    std::vector<TriangleT> faces_a; //{mesh_a->n_faces()};
    std::vector<TriangleT> faces_b; //{mesh_b->n_faces()};
    faces_a.reserve(mesh_a->n_faces());
    faces_b.reserve(mesh_b->n_faces());

    // convert to array structure for faster access
    for (auto face: mesh_a->faces()) {
        v_index = 0;
        TriangleT triangle { PointT{}, PointT{}, PointT{} };
        for (auto vertex: mesh_a->vertices(face)) {
            pmp::Point point = mesh_a->position(vertex);
            triangle[v_index++] = PointT{ point[0], point[1], point[2] };
        }
        faces_a.emplace_back(triangle);
    }

    for (auto face: mesh_b->faces()) {
        v_index = 0;
        TriangleT triangle{ PointT{}, PointT{}, PointT{} };
        for (auto vertex: mesh_b->vertices(face)) {
            pmp::Point point = mesh_b->position(vertex);
            triangle[v_index++] = PointT{ point[0], point[1], point[2] };
        }
        faces_b.emplace_back(triangle);
    }

    // out variables for intersection
//    PointT int_start{};
//    PointT int_end{};
//    bool coplanar;

    // reset edge feature
    if (mesh_a->has_edge_property("e:feature")) {
        // convert rvalue to lvalue
        auto mesh_a_feature = mesh_a->get_edge_property<bool>("e:feature");
        mesh_a->remove_edge_property(mesh_a_feature);
    }
    if (mesh_b->has_edge_property("e:feature")) {
        auto mesh_b_feature = mesh_b->get_edge_property<bool>("e:feature");
        mesh_b->remove_edge_property(mesh_b_feature);
    }

    // intersection feature
    auto intersect_a = mesh_a->edge_property<bool>("e:feature", false);
    auto intersect_b = mesh_b->edge_property<bool>("e:feature", false);

    auto intersect_a_ignore = mesh_a->vertex_property<bool>("v:intersection_ignore", false);
    auto intersect_b_ignore = mesh_b->vertex_property<bool>("v:intersection_ignore", false);

    // check intersection for every triangle (= face)
    #pragma omp parallel for default (none) shared (mesh_a, mesh_b, faces_a, faces_b, intersect_a_ignore, intersect_b_ignore, intersect_a, intersect_b) reduction(+:intersections)
    for (size_t index_a = 0; index_a < mesh_a->n_faces(); ++index_a) {
        TriangleT& tri_a = faces_a[index_a];

        // ignore triangle (mesh a)?
        bool skip_tri_a = false;
        for (auto v : mesh_a->vertices(pmp::Face(index_a))) {
            if (intersect_a_ignore[v]) {
                skip_tri_a = true;
            }
        }
        if (skip_tri_a) {
            continue;
        }

        for (size_t index_b = 0; index_b < mesh_b->n_faces(); ++index_b) {
            TriangleT& tri_b = faces_b[index_b];
            //std::cout << "triA" << tri_a[0] << " tri_b" << tri_b[0] << std::endl;

            // ignore triangle (mesh b)?
            bool skip_tri_b = false;
            for (auto v : mesh_b->vertices(pmp::Face(index_b))) {
                if (intersect_b_ignore[v]) {
                    skip_tri_b = true;
                }
            }
            if (skip_tri_b) {
                continue;
            }

            // these are dummies, so create for every thread
            PointT int_start{};
            PointT int_end{};
            bool coplanar;

            if (igl::tri_tri_intersection_test_3d(
                    tri_a[0], tri_a[1], tri_a[2],
                    tri_b[0], tri_b[1], tri_b[2],
                    coplanar, int_start, int_end)
            ) {
//                intersect_a[mesh_a->edge(mesh_a->halfedge(pmp::Face(index_a)))] = true;
//                intersect_b[mesh_b->edge(mesh_b->halfedge(pmp::Face(index_b)))] = true;
                mark_intersection(mesh_a, intersect_a, pmp::Face(index_a));
                mark_intersection(mesh_b, intersect_b, pmp::Face(index_b));

                intersections++;
            } // intersection?
        } // triangles b
    } // triangles a

    // no intersection
    return intersections;
}

// ---------------------------------------------------------------------------------------------------------------------

// =====================================================================================================================
