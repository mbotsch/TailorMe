//-----------------------------------------------------------------------------

#include "MeshStitching.h"

#include <fstream>

#include <pmp/algorithms/differential_geometry.h>
#include <pmp/io/io.h>

#include <Eigen/Dense>
#include <Eigen/Sparse>

#include "utils/io/io_selection.h"

//-----------------------------------------------------------------------------

Mesh_stitcher::Mesh_stitcher()
{
}


//-----------------------------------------------------------------------------


void Mesh_stitcher::init(pmp::SurfaceMesh& mesh,
                         const std::string& fn_locked_vertices)
{
    cleanup();

    std::vector<int> locked_vertices;
    read_selection(fn_locked_vertices, locked_vertices);

    mesh_ = &mesh,
    num_free_vertices_ = mesh_->n_vertices();

    save_undeformed_state(locked_vertices);

    init_ = true;
}


//-----------------------------------------------------------------------------


void Mesh_stitcher::cleanup()
{
    using namespace pmp;

    if (!mesh_)
    {
        // No need for cleanup, never initialized
        return;
    }

    // Delete any properties created
    {
        // Vertex properties with type Point
        auto prop = mesh_->get_vertex_property<Point>("v:stitching_undeformed");
        mesh_->remove_vertex_property(prop);

        prop = mesh_->get_vertex_property<Point>("v:stitching_deformed_laplace");
        mesh_->remove_vertex_property(prop);

        prop = mesh_->get_vertex_property<Point>("v:stitching_undeformed_laplace");
        mesh_->remove_vertex_property(prop);

        prop = mesh_->get_vertex_property<Point>("v:stitching_target_laplace");
        mesh_->remove_vertex_property(prop);
    }

    {
        // Other properties
        auto prop1 = mesh_->get_vertex_property<double>("v:stitching_weight");
        mesh_->remove_vertex_property(prop1);

        auto prop2 = mesh_->get_edge_property<double>("e:stitching_weight");
        mesh_->remove_edge_property(prop2);

        auto prop3 = mesh_->get_vertex_property<int>("v:stitching_idx");
        mesh_->remove_vertex_property(prop3);

        auto prop4 = mesh_->get_vertex_property<bool>("v:stitching_locked");
        mesh_->remove_vertex_property(prop4);
    }
}


//-----------------------------------------------------------------------------


Mesh_stitcher::~Mesh_stitcher()
{
    cleanup();
}

//-----------------------------------------------------------------------------

void Mesh_stitcher::save_undeformed_state(std::vector<int>& locked_vertices)
{
    using namespace pmp;

    // Backup undeformed points
    auto points = mesh_->vertex_property<Point>("v:stitching_undeformed");
    points.vector() = mesh_->positions();

    // Laplace weights of undeformed state
    auto v_weight = mesh_->vertex_property<double>("v:stitching_weight");
    auto e_weight = mesh_->edge_property<double>("e:stitching_weight");

    for (auto v : mesh_->vertices())
        v_weight[v] = voronoi_area(*mesh_, v);

    for (auto e : mesh_->edges())
        e_weight[e] = cotan_weight(*mesh_, e);

    // Laplace vectors of undeformed state
    compute_laplace("v:stitching_undeformed_laplace");

    // Locked vertices
    auto is_locked = mesh_->vertex_property<bool>("v:stitching_locked", false);
    auto idx       = mesh_->vertex_property<int>("v:stitching_idx", -1);

    for (int idx : locked_vertices)
    {
        is_locked[Vertex(idx)] = true;
    }

    int i = 0;
    for (auto v : mesh_->vertices())
    {
        if (!is_locked[v])
        {
            idx[v] = i++;
        }
    }

    num_free_vertices_ = i;

    if ((size_t)num_free_vertices_ >= mesh_->n_vertices())
    {
        std::cerr << "[ERROR] Mesh_stitcher: Number of free vertices >= vertex count" << std::endl;
        std::cerr << "[ERROR] Mesh_stitcher: " << num_free_vertices_ << " >= " << mesh_->n_vertices() << std::endl;
        exit(-1);
    }
}

//-----------------------------------------------------------------------------

void Mesh_stitcher::compute_laplace(const std::string& property_name)
{
    using namespace pmp;

    auto points = mesh_->get_vertex_property<Point>("v:point");
    auto v_weight = mesh_->get_vertex_property<double>("v:stitching_weight");
    auto e_weight = mesh_->get_edge_property<double>("e:stitching_weight");

    auto laplace = mesh_->vertex_property<Point>(property_name);

    for (auto v : mesh_->vertices())
    {
        Point l(0, 0, 0);

        if (!mesh_->is_isolated(v))
        {
            double sum_weights = 0.0;
            for (auto h : mesh_->halfedges(v))
            {
                auto vv = mesh_->to_vertex(h);
                auto e = mesh_->edge(h);
                auto p = points[vv];
                auto w = e_weight[e];

                l += w * p;
                sum_weights += w;
            }

            Point p = mesh_->position(v);
            l -= (sum_weights * p);
            l *= v_weight[v];
        }

        laplace[v] = l;
    }
}

//-----------------------------------------------------------------------------

void Mesh_stitcher::stitch()
{
    using namespace pmp;

    if (! init_)
    {
        return;
    }

    // Laplace vectors of deformed state
    auto undeformed_laplace = mesh_->get_vertex_property<Point>("v:stitching_undeformed_laplace");
    auto v_weight = mesh_->get_vertex_property<double>("v:stitching_weight");
    auto e_weight = mesh_->get_edge_property<double>("e:stitching_weight");

    // Laplace vectors of deformed state
    compute_laplace("v:stitching_deformed_laplace");
    auto deformed_laplace = mesh_->get_vertex_property<Point>("v:stitching_deformed_laplace");

    // Reuse undeformed property to save mesh positions in unstitched state
    auto unstitched = mesh_->get_vertex_property<Point>("v:stitching_undeformed");
    unstitched.vector() = mesh_->positions();

    // Deformed vertex positions
    auto points = mesh_->get_vertex_property<Point>("v:point");

    // Vertex properties useful for setting up LSE
    auto is_locked = mesh_->get_vertex_property<bool>("v:stitching_locked");
    auto idx       = mesh_->get_vertex_property<int>("v:stitching_idx");

    // Setup target laplace
    auto target_laplace = mesh_->vertex_property<Point>("v:stitching_target_laplace");
    for (auto v : mesh_->vertices())
    {
        bool neighbor_is_locked = false;
        for (auto neighbor : mesh_->vertices(v))
            if (is_locked[neighbor])
                neighbor_is_locked = true;

        if (is_locked[v] || neighbor_is_locked)
            target_laplace[v] = deformed_laplace[v];
        else
            target_laplace[v] = undeformed_laplace[v];
    }

    // Setup LSE
    Eigen::SparseMatrix<double> L;
    Eigen::MatrixXd B;

    using Triplet = Eigen::Triplet<double>;

    L.resize(num_free_vertices_, num_free_vertices_);
    B.resize(num_free_vertices_, 3);

    std::vector<Triplet> triplets;

    for (auto v_i : mesh_->vertices())
    {
        if (is_locked[v_i])
            continue;

        auto i = idx[v_i];
        auto l = target_laplace[v_i];

        B(i, 0) = l[0] / v_weight[v_i];
        B(i, 1) = l[1] / v_weight[v_i];
        B(i, 2) = l[2] / v_weight[v_i];

        double sum_weights = 0.0;
        for (auto he : mesh_->halfedges(v_i))
        {
            auto v_j = mesh_->to_vertex(he);
            auto x_j = points[v_j];
            auto j = idx[v_j];
            auto e = mesh_->edge(he);

            auto e_weight_e = e_weight[e];
            sum_weights += e_weight_e;

            if (is_locked[v_j])
            {
                // rhs
                B(i, 0) -= e_weight_e * x_j[0];
                B(i, 1) -= e_weight_e * x_j[1];
                B(i, 2) -= e_weight_e * x_j[2];
            }
            else
            {
                triplets.push_back(Triplet(i, j, e_weight_e));
            }
        }

        triplets.push_back(Triplet(i, i, -sum_weights));
    }

    L.setFromTriplets(triplets.begin(), triplets.end());

    // Solve LSE
    Eigen::SimplicialLDLT<Eigen::SparseMatrix<double>> ldlt(L);
    Eigen::MatrixXd X = ldlt.solve(B);

    // Copy solution to mesh
    int i = 0;
    for (auto v : mesh_->vertices())
    {
        if (!is_locked[v])
        {
            Point p = Point(X(i, 0), X(i, 1), X(i, 2));
            mesh_->position(v) = p;

            i++;
        }
    }

    auto stitched = mesh_->get_vertex_property<Point>("v:point");

    // Calculate best transform from unstitched to stitched state
    // (e.g. for transforming point cloud accordingly)
    std::vector<Point> vertices_unstitched;
    std::vector<Point> vertices_stitched;

    vertices_unstitched.reserve(num_free_vertices_);
    vertices_stitched.reserve(num_free_vertices_);

    for (auto v : mesh_->vertices())
    {
        bool neighbor_is_locked = false;

        for (auto v_j : mesh_->vertices(v))
            if (is_locked[v_j])
                neighbor_is_locked = true;

        if (!is_locked[v] && !neighbor_is_locked)
        {
            vertices_unstitched.push_back(unstitched[v]);
            vertices_stitched.push_back(stitched[v]);
        }
    }
}

//-----------------------------------------------------------------------------
