set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/filesystem_utils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ndarray_io.h
    ${CMAKE_CURRENT_SOURCE_DIR}/io_selection.h
    ${CMAKE_CURRENT_SOURCE_DIR}/io_vertexweighting.h
    ${CMAKE_CURRENT_SOURCE_DIR}/pmp_io.h
)

set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/filesystem_utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ndarray_io.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/io_vertexweighting.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pmp_io.cpp
)

target_sources(${PROJECT_NAME} PRIVATE ${HEADERS} ${SOURCES})
