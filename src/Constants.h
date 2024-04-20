//======================================================================================================================
// Copyright (c) by Fabian Kemper 2024
//
// This work is licensed under a
// Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//
// You should have received a copy of the license along with this
// work. If not, see <http://creativecommons.org/licenses/by-nc-sa/4.0/>.
//
//======================================================================================================================

#ifndef TAILORME_VIEWER_CONSTANTS_H
#define TAILORME_VIEWER_CONSTANTS_H


// folder for static data
const std::string RESOURCE_DATA_DIR = "data";
const std::string MODEL_DATA_DIR = "models";

// matcap files
const std::string MATCAP_BONE = "matcap-bone.jpg";
const std::string MATCAP_SKIN = "matcap-skin.jpg";

// for debugging
#define SKEL_HIRES_VERTICES 36860
#define SKEL_FULL_WRAP_VERTICES 23752
#define SKEL_PARTIAL_WRAP_VERTICES 13391
#define SKIN_VERTICES 23752


#endif //TAILORME_VIEWER_CONSTANTS_H
