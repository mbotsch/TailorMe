#ifndef TAILORME_VIEWER_POST_PROC_SMOOTH_H
#define TAILORME_VIEWER_POST_PROC_SMOOTH_H

// =====================================================================================================================

#include "post_processing_base.h"

// =====================================================================================================================

class PostProcessingSmoothing : public PostProcessingBase
{
  public:
    PostProcessingSmoothing();
    // perform smoothing
    auto postprocess(pmp::SurfaceMesh* skel, pmp::SurfaceMesh* skin) -> void override;

  private:
    std::vector<int> locked_indices_;
};

// =====================================================================================================================

#endif // TAILORME_VIEWER_POST_PROC_SMOOTH_H

// =====================================================================================================================
