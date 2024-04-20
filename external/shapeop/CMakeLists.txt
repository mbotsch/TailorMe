#file(GLOB_RECURSE SRCS ./*.cpp)
#file(GLOB_RECURSE HDRS ./*.h)

set(SRCS
    Constraint.cpp
    Force.cpp
    LSSolver.cpp
    Solver.cpp)

set(HDRS
    Common.h
    Constraint.h
    Force.h
    LSSolver.h
    Solver.h
    Types.h)

find_package(OpenMP)

add_library(shapeop ${SRCS})
if (OpenMP_CXX_FOUND)
    target_link_libraries(shapeop OpenMP::OpenMP_CXX)
endif ()
target_include_directories(shapeop PRIVATE ${PROJECT_SOURCE_DIR}/external/pmp-library/external/eigen-3.4.0/)
#add_warnings_to_target(shapeop)
