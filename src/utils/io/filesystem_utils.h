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

#ifndef TAILORME_VIEWER_FILESYSTEM_UTILS_H
#define TAILORME_VIEWER_FILESYSTEM_UTILS_H

#include <string>

class FilesystemUtils {
  public:
    // does a file exist?
    static auto file_exists(const std::string& filename) -> bool;

    // basename from path
    static auto filename(const std::string& filename) -> std::string;
};

#endif // TAILORME_VIEWER_FILESYSTEM_UTILS_H
