set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/post_processing_base.h
    ${CMAKE_CURRENT_SOURCE_DIR}/post_proc_face_mirror.h
    ${CMAKE_CURRENT_SOURCE_DIR}/post_proc_smoothing.h
)

set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/post_processing_base.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/post_proc_face_mirror.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/post_proc_smoothing.cpp
)

target_sources(${PROJECT_NAME} PRIVATE ${HEADERS} ${SOURCES})

