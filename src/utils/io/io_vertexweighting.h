#pragma once

#include <filesystem>
#include <string>

#include <pmp/surface_mesh.h>

bool load_vertexweighting(const std::filesystem::path& filename, pmp::SurfaceMesh& mesh, const std::string& property_name);
