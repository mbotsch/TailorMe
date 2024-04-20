//-----------------------------------------------------------------------------

#pragma once

#include <pmp/surface_mesh.h>

//-----------------------------------------------------------------------------

class Mesh_stitcher
{
public:
    Mesh_stitcher();
    ~Mesh_stitcher();

    void init(pmp::SurfaceMesh& mesh, const std::string& fn_locked_vertices);

    void stitch();

private:
    void save_undeformed_state(std::vector<int>& locked_vertices);

    void compute_laplace(const std::string& property_name);

    void cleanup();

    pmp::SurfaceMesh* mesh_ = nullptr;
    int num_free_vertices_;

    bool init_ = false;
};

//-----------------------------------------------------------------------------

