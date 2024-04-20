//
// Created by fabian on 9/24/23.
//
// =====================================================================================================================

#include "post_proc_smoothing.h"

#include <pmp/algorithms/laplace.h>

#include "utils/io/io_selection.h"
#include "Constants.h"

// =====================================================================================================================

// ---------------------------------------------------------------------------------------------------------------------

PostProcessingSmoothing::PostProcessingSmoothing()
{
    locked_indices_.clear();
    if (!read_selection(RESOURCE_DATA_DIR + "/bo_head_hands_toes.sel", locked_indices_))
    {
        std::cerr << "[ERROR] Cannot load locked vertex selection from " << RESOURCE_DATA_DIR << "/bo_head_hands_toes.sel" << std::endl;
    }
}

// ---------------------------------------------------------------------------------------------------------------------

auto explicit_smoothing_selected(pmp::SurfaceMesh& mesh, unsigned int iters,
                                 const std::vector<int>& locked_indices,
                                 bool use_uniform_laplace) -> void
{
    // NOTE:
    //   Function is copied from pmp explicit smoothing and modified
    //   to support control of locked vertices.

    using namespace pmp;

    if (!mesh.n_vertices())
        return;

    auto is_locked = mesh.vertex_property<bool>("v:explicit_smoothing_is_locked", false);
    for (auto v : mesh.vertices())
    {
        if (mesh.is_boundary(v)) is_locked[v] = true;
    }

    for (int idx : locked_indices)
    {
        is_locked[Vertex(idx)] = true;
    }

    // Laplace matrix (clamp negative cotan weights to zero)
    SparseMatrix L;
    if (use_uniform_laplace) {
        uniform_laplace_matrix(mesh, L);
    } else {
        laplace_matrix(mesh, L, true);
    }

    // normalize each row by sum of weights
    // scale by 0.5 to make it more robust
    // multiply by -1 to make it neg. definite again
    L = -0.5 * L.diagonal().asDiagonal().inverse() * L;

    // cancel out boundary vertices
    SparseMatrix S;
    auto is_inner = [&](Vertex v) { return !is_locked[v]; };
    selector_matrix(mesh, is_inner, S);
    L = S.transpose() * S * L;

    // copy vertex coordinates to matrix
    DenseMatrix X;
    coordinates_to_matrix(mesh, X);

    // perform some interations
    for (unsigned int i = 0; i < iters; ++i) {
        X += L * X;
    }

    // copy matrix back to vertex coordinates
    matrix_to_coordinates(X, mesh);

    mesh.remove_vertex_property(is_locked);
}


// ---------------------------------------------------------------------------------------------------------------------

auto PostProcessingSmoothing::postprocess(pmp::SurfaceMesh* skel, pmp::SurfaceMesh* skin) -> void
{
    (void) skel;


    explicit_smoothing_selected(*skin, 1, locked_indices_, /*use_uniform_laplace=*/ true);

    //pmp::explicit_smoothing(*skin, 1, /*use_uniform_laplace=*/true);
//    pmp::implicit_smoothing(*skin, 1.0, /*use_uniform_laplace=*/true, /*rescale=*/false);
}

// ---------------------------------------------------------------------------------------------------------------------


// =====================================================================================================================
