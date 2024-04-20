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

#ifndef TAILORME_VIEWER_POST_PROC_FACE_MIRROR_H
#define TAILORME_VIEWER_POST_PROC_FACE_MIRROR_H

// =====================================================================================================================

#include "post_processing_base.h"

// =====================================================================================================================

#define FACE_MIRROR_DATA_FILE "head_mirror_weights.mat"

// =====================================================================================================================

class PostProcessingFaceMirror : public PostProcessingBase
{
  protected:
    // strength of blending 0.0 = no filter applied
    // 1.0 = complete mirror
    float _blend_rate = 0.75F;

    // blending data format
    // vertex1, vertex2, weight
    MatrixXf _blending_data {};

    // blend points
    static auto blend_points(pmp::Point& point1, pmp::Point& point2, const float& mix_ratio) -> void;

  public:
    PostProcessingFaceMirror();

    // perform mirroring
    auto postprocess(pmp::SurfaceMesh* skel, pmp::SurfaceMesh* skin) -> void override;
};

// =====================================================================================================================

#endif // TAILORME_VIEWER_POST_PROC_FACE_MIRROR_H

// =====================================================================================================================
