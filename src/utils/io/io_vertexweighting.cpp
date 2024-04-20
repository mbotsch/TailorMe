#include "io_vertexweighting.h"

#include <fstream>

bool load_vertexweighting(const std::filesystem::path& filename, pmp::SurfaceMesh& mesh, const std::string& property_name)
{
    std::ifstream ifs(filename);
    if (!ifs)
    {
        fprintf(stderr, "[ERROR] load_vertexweighting: Cannot open %s\n", filename.c_str());
        return false;
    }

    auto vw = mesh.vertex_property<float>(property_name);

    for (auto v : mesh.vertices())
    {
        ifs >> vw[v];

        if (!ifs.good())
        {
            fprintf(stderr, "[ERROR] load_vertexweighting: not enough values specified in %s\n", filename.c_str());
            mesh.remove_vertex_property(vw);
            return false;
        }
    }

    return true;
}
