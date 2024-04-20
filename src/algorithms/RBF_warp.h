//-----------------------------------------------------------------------------

#pragma once

#include <vector>

#include <Eigen/Dense>

#include <pmp/surface_mesh.h>

//-----------------------------------------------------------------------------

struct RBF_data
{
    std::vector<pmp::dvec3> centers;
    std::vector<int> indices;
    std::vector<int> mapping_full_to_cutoff;
    int num_centers;
    Eigen::PartialPivLU<Eigen::MatrixXd> solver;
};

//-----------------------------------------------------------------------------

bool init_rbf_warp(pmp::SurfaceMesh& skel_wrap,
                   size_t num_centers,
                   RBF_data& out_data);

//-----------------------------------------------------------------------------

bool init_rbf_warp_prioritize_head(pmp::SurfaceMesh& skel_wrap,
                                   size_t num_additional_centers,
                                   RBF_data& out_data);

//-----------------------------------------------------------------------------

bool apply_rbf_warp(pmp::SurfaceMesh& skel_wrap, pmp::SurfaceMesh& bones, RBF_data& rbf_data);

//-----------------------------------------------------------------------------
