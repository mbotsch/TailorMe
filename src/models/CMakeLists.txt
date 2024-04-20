set(HEADERS
        ${CMAKE_CURRENT_SOURCE_DIR}/BaseModel.h
        ${CMAKE_CURRENT_SOURCE_DIR}/SpiralNetAEModel.h
)

set(SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/BaseModel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/SpiralNetAEModel.cpp
)

target_sources(${PROJECT_NAME} PRIVATE ${HEADERS} ${SOURCES})
