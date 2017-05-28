cmake_minimum_required(VERSION 2.8)

project(fall)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR/build})
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(EXTERNAL_LIBS_PATH ${PROJECT_SOURCE_DIR}/libs/bin)

include_directories("${PROJECT_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}/libs/include")
find_library(SDL2MAIN_LIB SDL2main "${PROJECT_SOURCE_DIR}/libs/bin")
find_library(SDL2_LIB SDL2 "${PROJECT_SOURCE_DIR}/libs/bin")
find_library(GLEW32_LIB glew32s "${PROJECT_SOURCE_DIR}/libs/bin")
add_executable(fall ${PROJECT_SOURCE_DIR}/fall.cpp)
target_link_libraries(fall openGL32 ${SDL2MAIN_LIB} ${SDL2_LIB} ${GLEW32_LIB})
