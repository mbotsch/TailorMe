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

#include "filesystem_utils.h"

#include <filesystem>
#include <sys/stat.h>

// =====================================================================================================================

// ---------------------------------------------------------------------------------------------------------------------

auto FilesystemUtils::file_exists(const std::string &filename) -> bool
{
    struct stat buffer {};
    return (stat(filename.c_str(), &buffer) == 0);
    //    return ( access(filename.c_str(), F_OK) != -1 );
}

// ---------------------------------------------------------------------------------------------------------------------

auto FilesystemUtils::filename(const std::string& filename) -> std::string
{
    return std::filesystem::path(filename).filename();
}

// ---------------------------------------------------------------------------------------------------------------------


// =====================================================================================================================
