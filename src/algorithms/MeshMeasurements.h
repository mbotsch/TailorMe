#pragma once

#include <pmp/surface_mesh.h>

static float get_arm_length_boerner(const pmp::SurfaceMesh& mesh)
{
    using namespace pmp;

    const static std::array<int, 3> shoulder_elbow_wrist_vertex_ids =  {
        9978, 5884, 10626
    };

    float length_segment1 = distance(mesh.position(Vertex(shoulder_elbow_wrist_vertex_ids[0])),
                                     mesh.position(Vertex(shoulder_elbow_wrist_vertex_ids[1])));

    float length_segment2 = distance(mesh.position(Vertex(shoulder_elbow_wrist_vertex_ids[1])),
                                     mesh.position(Vertex(shoulder_elbow_wrist_vertex_ids[2])));

    return length_segment1 + length_segment2;
}
