//-----------------------------------------------------------------------------

#include "LayerCollisionResolve.h"

#include <set>
#include <fstream>
#include <pmp/stop_watch.h>
#include <pmp/io/io.h>

#include <shapeop/Solver.h>
#include <shapeop/Constraint.h>

#include "algorithms/TriTriIntersect.h"

//-----------------------------------------------------------------------------

bool resolve_layer_intersections_by_bottom_layer(pmp::SurfaceMesh& top_layer,
                                                 pmp::SurfaceMesh& bottom_layer,
                                                 pmp::VertexProperty<bool> locked)
{
    if (!locked)
    {
        printf("[ERROR] resolve_layer_intersections(): No locked vertices defined\n");
        return false;
    }
    int max_iter = 30;
    int iter = 0;

    // List of faces to check, we are only interested in faces, where all vertices are free
    // TODO(swenninger): Speed - this could be done once up front
    std::vector<pmp::Face> faces_to_check;
    for (auto f : top_layer.faces())
    {
        // bool any_locked = false;
        bool all_locked = true;
        for (auto v : top_layer.vertices(f))
        {
            if (locked[v])
            {
                // any_locked = true;
            }
            else
            {
                all_locked = false;
            }
        }

        if (!all_locked)
            faces_to_check.push_back(f);
    }

    std::vector<std::vector<pmp::Face>> neighbors_to_check(faces_to_check.size());

    #pragma omp parallel for
    for (size_t i = 0; i < faces_to_check.size(); ++i)
    {
        auto f = faces_to_check[i];

        std::vector<pmp::Vertex> v_neighbors;
        std::set<pmp::Face> f_neighbors;

        // Seperating axes test
        Eigen::RowVector3f p1, q1, r1;
        Eigen::RowVector3f p2, q2, r2;

        auto viter = top_layer.vertices(f);

        //v_neighbors.insert(*viter);
        v_neighbors.push_back(*viter);

        for (int j = 0; j < 2; ++j)
        {
            std::vector<pmp::Vertex> new_vs;
            for (auto vi : v_neighbors)
            {
                for (auto vj : top_layer.vertices(vi))
                {
                    new_vs.push_back(vj);
                }
            }

            for (auto vi : new_vs)
            {
                // v_neighbors.insert(vi);
                v_neighbors.push_back(vi);
            }
        }

        for (auto vi : v_neighbors)
        {
            for (auto fi : top_layer.faces(vi))
            {
                f_neighbors.insert(fi);
            }
        }

        for (auto fi : f_neighbors)
        {
            neighbors_to_check[i].push_back(fi);
        }
    }

    // Build mapping from vertex index to index in shapeop simulation
    // TODO(swenninger): Speed - this could be done once up front
    auto sim_idx = top_layer.vertex_property<int>("v:collision_sim_idx");

    int num_sim_vertices = 0;
    {
        int idx = 0;
        for (auto v : top_layer.vertices())
        {
            if (!locked[v])
            {
                sim_idx[v] = idx++;
            }
            else
            {
                // Check if we have a free neighbor
                bool all_neighbors_locked = true;

                for (auto vj : top_layer.vertices(v))
                {
                    if (!locked[vj])
                    {
                        all_neighbors_locked = false;
                        break;
                    }
                }

                if (all_neighbors_locked)
                {
                    sim_idx[v] = -1;
                }
                else
                {
                    sim_idx[v] = idx++;
                }
            }
        }

        num_sim_vertices = idx;
    }

    ShapeOp::Matrix3X points(3, num_sim_vertices);
    std::vector<bool> non_colliding(num_sim_vertices, true);
    auto collision_n = top_layer.vertex_property<pmp::dvec3>("v:collision_n");


    auto efeature = top_layer.edge_property<bool>("e:feature", false);
    auto f_collides = top_layer.face_property<bool>("f:collides", false);

    while (iter < max_iter)
    {
        int n_collisions = 0;

        // Reset collision state
        non_colliding.assign(num_sim_vertices, true);
        efeature.vector().assign(top_layer.n_edges(), false);
        f_collides.vector().assign(top_layer.n_faces(), false);

        std::vector<pmp::Face> colliding_faces = {};

        Eigen::RowVector3f collision_start;
        Eigen::RowVector3f collision_end;

        #pragma omp parallel for
        for (size_t i = 0; i < faces_to_check.size(); ++i)
        {
            auto f = faces_to_check[i];

            // Seperating axes test
            Eigen::RowVector3f p1, q1, r1;
            Eigen::RowVector3f p2, q2, r2;

            auto viter = top_layer.vertices(f);

            p1 = (Eigen::Vector3f)top_layer.position(*viter);
            p2 = (Eigen::Vector3f)bottom_layer.position(*viter);
            ++viter;
            q1 = (Eigen::Vector3f)top_layer.position(*viter);
            q2 = (Eigen::Vector3f)bottom_layer.position(*viter);
            ++viter;
            r1 = (Eigen::Vector3f)top_layer.position(*viter);
            r2 = (Eigen::Vector3f)bottom_layer.position(*viter);

            // Eigen::RowVector3f n_top    = (p1 - r1).cross(q1 - r1).normalized();
            Eigen::RowVector3f n_bottom = (p2 - r2).cross(q2 - r2).normalized();

            for (auto fi : neighbors_to_check[i])
            {
                viter = top_layer.vertices(fi);
                p2 = (Eigen::Vector3f)bottom_layer.position(*viter);
                ++viter;
                q2 = (Eigen::Vector3f)bottom_layer.position(*viter);
                ++viter;
                r2 = (Eigen::Vector3f)bottom_layer.position(*viter);

                bool coplanar;
                if (igl::tri_tri_intersection_test_3d(p1, q1, r1, p2, q2, r2, coplanar, collision_start, collision_end))
                {
                    n_collisions++;

                    f_collides[f] = true;

                    for (auto v : top_layer.vertices(f))
                    {
                        non_colliding[sim_idx[v]] = false;
                        collision_n[v] = (Eigen::Vector3f)n_bottom;
                    }

                    break;
                }
            }
        }

        if (n_collisions == 0)
        {
            break;
        }

        // Allow 'n_rings'-ring-neighborhood of moving layer vertices to move also
        {
            int n_rings = 2;
            for (int j = 0; j < n_rings; ++j)
            {
                std::vector<pmp::Vertex> new_vs;
                for (pmp::Vertex vi : top_layer.vertices())
                {
                    if (!non_colliding[sim_idx[vi]])
                    {
                        for (pmp::Vertex vj : top_layer.vertices(vi))
                        {
                            if (!locked[vj])
                            {
                                new_vs.push_back(vj);
                            }
                        }
                    }
                }

                for (pmp::Vertex v : new_vs)
                {
                    non_colliding[sim_idx[v]] = false;
                }
            }
        }

        // Gather points
        {
            for (pmp::Vertex v : bottom_layer.vertices())
            {
                if (sim_idx[v] != -1)
                {
                    points.col(sim_idx[v]) = (ShapeOp::Vector3)bottom_layer.position(v);
                }
            }
        }

        ShapeOp::Solver so_solver;
        so_solver.setPoints(points);

        // Keep almost all vertices fixed
        std::vector<int> vertex_id(1, -1);
        for (auto v : top_layer.vertices())
        {
            if (sim_idx[v] == -1)
                continue;

            vertex_id[0] = sim_idx[v];

            if (non_colliding[vertex_id[0]])
            {
                double weight = locked[v] ? 100.0 : 1.0;

                auto c = std::make_shared<ShapeOp::ClosenessConstraint>
                    (vertex_id, weight, so_solver.getPoints());

                so_solver.addConstraint(c);
            }
        }

        int n_collision_constraints = 0;
        for (pmp::Vertex v : top_layer.vertices())
        {
            if (sim_idx[v] == -1)
                continue;

            vertex_id[0] = sim_idx[v];

            if (!non_colliding[vertex_id[0]])
            {
                pmp::Point p_top = top_layer.position(v);

                // Successively add weight and collision_resolve_distance,
                // if it does not succeed in previous iterations
                double weight = (iter + 1) * 50.0;
                double dist_to_move_in_collision_n = 0.0025; // + iter * 0.0005;

                auto c = std::make_shared<ShapeOp::PlaneCollisionConstraint>
                    (vertex_id, weight, so_solver.getPoints(), p_top, collision_n[v], dist_to_move_in_collision_n);

                so_solver.addConstraint(c);
                n_collision_constraints++;
            }
        }

        // Regularize triangle shape of top layer
        std::vector<int> triangle_pair_ids(4, -1);

        // TODO(swenninger): Speed - we could build a vector of triangle_pair_ids up front,
        //                   since we always add bending constraings for the same faces.
        //                   Then the halfedge/to_vertex/next_halfedge topology lookups would be gone.
        for (pmp::Edge e : bottom_layer.edges())
        {
            pmp::Halfedge h0 = bottom_layer.halfedge(e, 0);
            pmp::Halfedge h1 = bottom_layer.halfedge(e, 1);

            pmp::Vertex v0 = bottom_layer.to_vertex(h0);
            pmp::Vertex v1 = bottom_layer.to_vertex(h1);
            pmp::Vertex v2 = bottom_layer.to_vertex(bottom_layer.next_halfedge(h0));
            pmp::Vertex v3 = bottom_layer.to_vertex(bottom_layer.next_halfedge(h1));

            if (locked[v0] || locked[v1] || locked[v2] || locked[v3])
            {
                continue;
            }

            triangle_pair_ids[0] = sim_idx[v0];
            triangle_pair_ids[1] = sim_idx[v1];
            triangle_pair_ids[2] = sim_idx[v2];
            triangle_pair_ids[3] = sim_idx[v3];

            auto c = std::make_shared<ShapeOp::BendingConstraint>
                (triangle_pair_ids, 1.0, so_solver.getPoints(), 0.9, 1.1);

            so_solver.addConstraint(c);
        }

        if (!so_solver.initialize())
        {
            printf("[ERROR] Cannot initialize shape op solver\n");
        }

        if (!so_solver.solve(5))
        {
            printf("[ERROR] Cannot solve with shape op solver\n");
        }

        const ShapeOp::Matrix3X& result_points = so_solver.getPoints();
        {
            for (pmp::Vertex v : bottom_layer.vertices())
            {
                if (sim_idx[v] != -1)
                {
                    bottom_layer.position(v) = (pmp::vec3)result_points.col(sim_idx[v]);
                }
            }
        }

        iter++;
    }

    top_layer.remove_face_property(f_collides);
    top_layer.remove_vertex_property(collision_n);
    top_layer.remove_vertex_property(sim_idx);

    return false;
}

//-----------------------------------------------------------------------------
