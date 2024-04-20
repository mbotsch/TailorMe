set(HEADERS
        ${CMAKE_CURRENT_SOURCE_DIR}/LayerCollisionResolve.h
    ${CMAKE_CURRENT_SOURCE_DIR}/MeshIntersection.h
    ${CMAKE_CURRENT_SOURCE_DIR}/MeshMeasurements.h
    ${CMAKE_CURRENT_SOURCE_DIR}/MeshStitching.h
    ${CMAKE_CURRENT_SOURCE_DIR}/RBF_warp.h
    ${CMAKE_CURRENT_SOURCE_DIR}/TriTriIntersect.h
)

set(SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/LayerCollisionResolve.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/MeshIntersection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/MeshStitching.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RBF_warp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/TriTriIntersect.cpp
)

target_sources(${PROJECT_NAME} PRIVATE ${HEADERS} ${SOURCES})
