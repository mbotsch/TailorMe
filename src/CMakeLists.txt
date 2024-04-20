
set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/Constants.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Globals.h
    ${CMAKE_CURRENT_SOURCE_DIR}/GlobTypes.h
    ${CMAKE_CURRENT_SOURCE_DIR}/TailorMeViewer.h
)

set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Globals.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/TailorMeViewer.cpp
)

target_sources(${PROJECT_NAME} PRIVATE ${HEADERS} ${SOURCES})

# add subdirectories
add_subdirectory(algorithms)
add_subdirectory(mesh_massage)
add_subdirectory(meshes)
add_subdirectory(models)
add_subdirectory(utils)
