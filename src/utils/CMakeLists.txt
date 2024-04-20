set(HEADERS
        ${CMAKE_CURRENT_SOURCE_DIR}/name_utils.h
)

set(SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/name_utils.cpp
)

target_sources(${PROJECT_NAME} PRIVATE ${HEADERS} ${SOURCES})

add_subdirectory(io)
