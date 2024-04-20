//======================================================================================================================
// Copyright (c) Fabian Kemper 2024
//
// This work is licensed under a
// Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//
// You should have received a copy of the license along with this
// work. If not, see <http://creativecommons.org/licenses/by-nc-sa/4.0/>.
//
//======================================================================================================================

#ifndef TAILORME_WRITEOBJ_H
#define TAILORME_WRITEOBJ_H

#include <filesystem>

#include <pmp/surface_mesh.h>

void read_obj_buffer(pmp::SurfaceMesh& mesh, const std::stringstream& buffer);


#endif //TAILORME_WRITEOBJ_H
