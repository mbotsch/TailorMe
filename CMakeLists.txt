cmake_minimum_required(VERSION 3.22 FATAL_ERROR)
project(tailorme_viewer VERSION 1.0)

# set cxx standard (nvidia says its only compatible with c++14 ... but compiles with c++17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set Compiler Options

# gcc
if(CMAKE_COMPILER_IS_GNUCC)
    # c++17 does not need this anymore?
    # https://gitlab.kitware.com/cmake/cmake/-/issues/17834
    #set(CXX_LIBRARIES "stdc++fs")
    message(STATUS "GCC: Found.")
    # require g++11 or newer
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 11)
        message(STATUS"GCC: HIT SLOW COMPILER")
        set(CMAKE_CXX_COMPILER "g++-11")
    endif ()
    # do not optimize too much
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")

    # static linking
    #link_directories(BEFORE "/usr/lib/gcc/x86_64-pc-linux-gnu/11.4.0")
    #set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
endif(CMAKE_COMPILER_IS_GNUCC)

# turn on (almost) all warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

# do not optimize for debug
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} -O0")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")

# windows fix for dynamic linked libpmp
if (MSVC)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
endif ()

# allow downloading git repositories
include(FetchContent)

### PMP Library
# compile PMP library - but skip all junk
set(PMP_BUILD_APPS     OFF CACHE BOOL "")
set(PMP_BUILD_EXAMPLES OFF CACHE BOOL "")
set(PMP_BUILD_TESTS    OFF CACHE BOOL "")
set(PMP_BUILD_DOCS     OFF CACHE BOOL "")
add_subdirectory(external/pmp-library EXCLUDE_FROM_ALL)
# include sources
include_directories(${PROJECT_SOURCE_DIR}/external/pmp-library/src EXCLUDE_FROM_ALL)
include_directories(${PROJECT_SOURCE_DIR}/external/pmp-library/external/imgui-1.89.5 EXCLUDE_FROM_ALL)

include_directories(${PROJECT_SOURCE_DIR}/external/pmp-library/external/eigen-3.4.0 EXCLUDE_FROM_ALL)
#set(Eigen3_DIR ${PROJECT_SOURCE_DIR}/external/pmp-library/external/eigen-3.4.0/cmake)
#find_package(Eigen3 REQUIRED NO_MODULE)

include_directories(${PROJECT_SOURCE_DIR}/external/pmp-library/external/glfw-3.3.8/include EXCLUDE_FROM_ALL)
include_directories(${PROJECT_SOURCE_DIR}/external/pmp-library/external/glew-2.2.0/include EXCLUDE_FROM_ALL)

### Shapeop Library
add_subdirectory(${PROJECT_SOURCE_DIR}/external/shapeop)
include_directories(${PROJECT_SOURCE_DIR}/external/pmp-library/external/)

set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

### PyTorch libraries
# compile with -DUSE_LOCAL_LIBS=on
option(USE_LOCAL_LIBS "Build with local pytorch version" ON)
option(USE_LOCAL_CONDA "Build with conda python version" OFF)

if(USE_LOCAL_LIBS)
    # download local versions for all libraries
    message(STATUS "Use local version of torch libraries.")
    get_filename_component(Torch_DIR "./external/libtorch/share/cmake/Torch" REALPATH)
    get_filename_component(TorchScatter_DIR "./external/pyg/share/cmake/TorchScatter" REALPATH)
    get_filename_component(TorchSparse_DIR "./external/pyg/share/cmake/TorchSparse" REALPATH)
    #set(Torch_DIR "external/libtorch/share/cmake/Torch")
    #set(TorchScatter_DIR "external/pyg/share/cmake/TorchScatter")
    #set(TorchSparse_DIR "external/pyg/share/cmake/TorchSparse")
    set(LIBTORCH_DIR "./external/libtorch" REALPATH)

    if(EMSCRIPTEN)
        get_filename_component(Torch_DIR "./external/libtorch-static/share/cmake/Torch" REALPATH)
        get_filename_component(TorchScatter_DIR "./external/pyg/share/cmake/TorchScatter" REALPATH)
        get_filename_component(TorchSparse_DIR "./external/pyg/share/cmake/TorchSparse" REALPATH)
        #set(Torch_DIR "external/libtorch/share/cmake/Torch")
        #set(TorchScatter_DIR "external/pyg/share/cmake/TorchScatter")
        #set(TorchSparse_DIR "external/pyg/share/cmake/TorchSparse")
        set(LIBTORCH_DIR "./external/libtorch-static" REALPATH)
        get_filename_component(Caffe2_DIR "./external/libtorch-static/share/cmake/Caffe2" REALPATH)
    endif(EMSCRIPTEN)
else()
    ### LibTorch -- use home dir libtorch if found
    set(LIBTORCH_DIR $ENV{HOME}/libtorch)

    ## set cuda architecture - should not be needed, if cuda command fails, there is likely a problem with your local cuda installation
    #if(NOT DEFINED ${CMAKE_CUDA_ARCHITECTURES})
    #    set(CMAKE_CUDA_ARCHITECTURES 52 61 75)
    #endif()
endif()

list(APPEND CMAKE_PREFIX_PATH ${LIBTORCH_DIR})

if(USE_LOCAL_CONDA)
    set(CMAKE_MODULE_PATH "$ENV{HOME}/miniconda3/envs/spirals")
    set(Python3_FIND_STRATEGY "LOCATION")
    set(Python3_DIR "$ENV{HOME}/miniconda3/envs/spirals/bin")
    set(Python3_EXECUTABLE "$ENV{HOME}/miniconda3/envs/spirals/bin/python3")
    # linux has no python 3.11 conda version for pytorch_scatter
    set(PYTHON_INCLUDE_PATH "$ENV{HOME}/miniconda3/envs/spirals/include/python3.10")
    # macosx has python 3.11 headers
    #set(PYTHON_INCLUDE_PATH "$ENV{HOME}/miniconda3/include/python3.11")
endif()

# for jit support
find_package(Python3 COMPONENTS Development)
# basic pytorch package
#set(CMAKE_CUDA_STANDARD 17)
#set(CMAKE_CUDA_FLAGS "-std=c++17")
find_package(Torch REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")

# needed for python_geometric support
#find_package(TorchScatter REQUIRED PATHS external/pytorch_scatter NO_DEFAULT_PATH)
find_package(TorchScatter REQUIRED)
find_package(TorchSparse REQUIRED)

# debug messages for cmake
message(STATUS "=== PyTorch Information ===")
message(STATUS "INCLUDE_DIRS=${TORCH_INCLUDE_DIRS};")
message(STATUS "LIBRARIES=${TORCH_LIBRARIES};")


# zstr - zip
find_package(libzip REQUIRED)
FetchContent_Declare(libzippp
    GIT_REPOSITORY    "https://github.com/ctabin/libzippp.git" # can also be a local filesystem path!
    GIT_TAG           "master"
)
FetchContent_MakeAvailable(libzippp) # defines INTERFACE target libzippp

# json package for meta files
find_package(nlohmann_json 3.2.0 REQUIRED)

# fmtlib
find_package(fmt)

# include argparse library
FetchContent_Declare(
        argparse
        GIT_REPOSITORY https://github.com/p-ranav/argparse.git
)
FetchContent_MakeAvailable(argparse)

# ImGui FileDialog
include_directories(${PROJECT_NAME} "${CMAKE_SOURCE_DIR}/external/ImGuiFileDialog")

# TODO: Find openMP on apple silicon automatically
# if (APPLE)
#   set(OpenMP_DIR ...)
# endif ()
find_package(OpenMP)

# set include directory (allow non relative imports)
include_directories(${PROJECT_NAME} "${CMAKE_SOURCE_DIR}/src")

# set headers
set(HEADERS
)

# set main sources
set(SOURCES
    external/ImGuiFileDialog/ImGuiFileDialog.cpp
    main.cpp
)

# set include directories
set(INCLUDE_DIRS
    ${TORCH_INCLUDE_DIRS}
    ${libzippp_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/external/
)

# set libraries
set(LIBRARIES
    ${CXX_LIBRARIES}
    ${TORCH_LIBRARIES}
    #Eigen3::Eigen
    pmp_vis
    libzippp
    shapeop
)

# main executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# important, add this before after main executable
add_subdirectory(src)

# include headers
target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDE_DIRS})

# add libraries to executable
target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBRARIES})
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt-header-only)
target_link_libraries(${PROJECT_NAME} PRIVATE argparse)

if (OpenMP_CXX_FOUND)
    message(STATUS "Found OpenMP.")
    target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC OpenMP::OpenMP_CXX)
endif ()

# apply c++std to main target
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD ${CMAKE_CXX_STANDARD})
set_property(TARGET ${PROJECT_NAME} PROPERTY CMAKE_CXX_STANDARD_REQUIRED ${CMAKE_CXX_STANDARD_REQUIRED})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_range_for)

# We now need to link the TorchScatter and TorchSparse libraries
# to our executable. We can do that by using the
# TorchScatter::TorchScatter and TorchSparse::TorchSparse targets,
# which also adds all the necessary torch dependencies.
target_link_libraries(${PROJECT_NAME} PRIVATE TorchScatter::TorchScatter)
target_link_libraries(${PROJECT_NAME} PRIVATE TorchSparse::TorchSparse)
target_link_libraries(${PROJECT_NAME} PRIVATE ${CUDA_cusparse_LIBRARY})

