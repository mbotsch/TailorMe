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

#ifndef TAILORME_VIEWER_NAME_UTILS_H
#define TAILORME_VIEWER_NAME_UTILS_H

#include "meshes/BaseMesh.h"

class NameUtils {
public:
    auto static mesh_type_str(MeshType mesh_type) -> std::string;
};


#endif // TAILORME_VIEWER_NAME_UTILS_H
