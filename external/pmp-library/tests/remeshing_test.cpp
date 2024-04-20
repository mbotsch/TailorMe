// Copyright 2017-2021 the Polygon Mesh Processing Library developers.
// Distributed under a MIT-style license, see LICENSE.txt for details.

#include "gtest/gtest.h"

#include "pmp/algorithms/remeshing.h"
#include "pmp/algorithms/features.h"
#include "pmp/algorithms/shapes.h"
#include "pmp/algorithms/triangulation.h"
#include "pmp/algorithms/utilities.h"

#include "helpers.h"

using namespace pmp;

// adaptive remeshing
TEST(RemeshingTest, adaptive_remeshing_with_features)
{
    auto mesh = cylinder();
    triangulate(mesh);
    detect_features(mesh, 25);
    auto bb = bounds(mesh).size();
    adaptive_remeshing(mesh, 0.001 * bb, // min length
                       1.0 * bb,         // max length
                       0.001 * bb);      // approx. error
    EXPECT_EQ(mesh.n_vertices(), 6u);
}

TEST(RemeshingTest, adaptive_remeshing_with_boundary)
{
    // mesh with boundary
    auto mesh = open_cone();
    auto bb = bounds(mesh).size();
    adaptive_remeshing(mesh, 0.01 * bb, // min length
                       1.0 * bb,        // max length
                       0.01 * bb);      // approx. error
    EXPECT_EQ(mesh.n_vertices(), size_t(104));
}

TEST(RemeshingTest, adaptive_remeshing_with_selection)
{
    auto mesh = icosphere(1);

    // select half the vertices
    auto selected = mesh.add_vertex_property<bool>("v:selected");
    for (auto v : mesh.vertices())
        if (mesh.position(v)[1] > 0)
            selected[v] = true;

    auto bb = bounds(mesh).size();
    adaptive_remeshing(mesh, 0.01 * bb, // min length
                       1.0 * bb,        // max length
                       0.01 * bb);      // approx. error
    EXPECT_EQ(mesh.n_vertices(), size_t(62));
}

TEST(RemeshingTest, uniform_remeshing)
{
    auto mesh = open_cone();
    uniform_remeshing(mesh, 0.5);
    EXPECT_EQ(mesh.n_vertices(), size_t(41));
}
