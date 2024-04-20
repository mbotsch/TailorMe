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
// =====================================================================================================================

#ifndef TAILORME_VIEWER_POSTPROCESSINGSTEP_H
#define TAILORME_VIEWER_POSTPROCESSINGSTEP_H

// =====================================================================================================================

#include "GlobTypes.h"
#include "pmp/surface_mesh.h"
#include <string>

// =====================================================================================================================

enum PostProcessingType {
    PostProcess_NONE,
    PostProcess_HEAD_SMOOTH,
};

class PostProcessingBase
{
  protected:
    bool _enabled = true;
    [[maybe_unused]]
    long _skel_vertex_count = 0;
    [[maybe_unused]]
    long _skin_vertex_count = 0;
  public:
    PostProcessingBase() = default;
    virtual ~PostProcessingBase() = default;

    virtual auto update_meshes(long skel_vertex_count, long skin_vertex_count) -> void;

    virtual auto get_processing_type() -> PostProcessingType;
    virtual auto set_enabled(bool enabled) -> void;
    virtual auto get_name() -> std::string;

    //virtual auto
    virtual auto postprocess(pmp::SurfaceMesh* skel, pmp::SurfaceMesh* skin) -> void;
};

// =====================================================================================================================

#endif // TAILORME_VIEWER_POSTPROCESSINGSTEP_H

// =====================================================================================================================
