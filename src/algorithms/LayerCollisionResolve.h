//-----------------------------------------------------------------------------

#pragma once

#include <pmp/surface_mesh.h>

//-----------------------------------------------------------------------------

bool resolve_layer_intersections_by_bottom_layer(pmp::SurfaceMesh& top_layer,
                                                 pmp::SurfaceMesh& bottom_layer,
                                                 pmp::VertexProperty<bool> locked);

//-----------------------------------------------------------------------------
