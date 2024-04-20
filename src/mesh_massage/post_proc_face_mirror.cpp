//
// Created by fabian on 9/21/23.
//
// =====================================================================================================================

#include "post_proc_face_mirror.h"

#include <filesystem>
#include <iostream>

#include "Globals.h"
#include "utils/io/ndarray_io.h"

// =====================================================================================================================

// ---------------------------------------------------------------------------------------------------------------------

PostProcessingFaceMirror::PostProcessingFaceMirror()
{
    auto vertex_filename = std::filesystem::path(globals::data_dir) / FACE_MIRROR_DATA_FILE;
    _blending_data = NDArray::open_matrix_f(vertex_filename);
}

// ---------------------------------------------------------------------------------------------------------------------

auto PostProcessingFaceMirror::blend_points(pmp::Point& point1, pmp::Point& point2, const float& mix_ratio) -> void
{
    // y-coordinate (up and down)
    float pos_y1 = point1[1];
    float pos_y2 = point2[1];
    point1[1] = mix_ratio * pos_y1 + (1.0F - mix_ratio) * pos_y2;
    point2[1] = mix_ratio * pos_y2 + (1.0F - mix_ratio) * pos_y1;
}

// ---------------------------------------------------------------------------------------------------------------------

auto PostProcessingFaceMirror::postprocess(pmp::SurfaceMesh* skel, pmp::SurfaceMesh* skin) -> void
{
    PostProcessingBase::postprocess(skel, skin);
    if (!_enabled) { return; }
    if ((skel->n_vertices() != 13391) || (skin->n_vertices() != 23752)){
        std::cout << "Point size wrong. Skip mirror filter.\n";
        return;
    }

    // blending strength between paired vertex (0 - 0.5) = 0.5 full mirror
    float mirror_strength = 1.0F - 0.5F * _blend_rate;

    for (long mirror_idx = 0; mirror_idx < _blending_data.cols(); ++mirror_idx) {
        long index1 = static_cast<long> (_blending_data(0, mirror_idx));
        long index2 = static_cast<long> (_blending_data(1, mirror_idx));

        if (index1 != index2) {
            // y value
            pmp::Point& point1 = skin->position(pmp::Vertex(index1));
            pmp::Point& point2 = skin->position(pmp::Vertex(index2));
            blend_points(point1, point2, mirror_strength);
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------

// =====================================================================================================================

