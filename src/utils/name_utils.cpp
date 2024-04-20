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

#include "name_utils.h"

auto NameUtils::mesh_type_str(MeshType mesh_type) -> std::string {
    std::string result;

    switch (mesh_type) {
        case MESH_UNDEFINED:
            result = "";
            break;
        case MESH_MALE:
            result = "male";
            break;
        case MESH_FEMALE:
            result = "female";
            break;
    }

    return result;
}
