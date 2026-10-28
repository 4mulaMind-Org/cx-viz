cmake_minimum_required(VERSION 3.28)
project(cx_viz LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(cx_core STATIC)
target_sources(cx_core PUBLIC FILE_SET CXX_MODULES FILES src/core.cppm)

add_library(cx_parse STATIC)
target_sources(cx_parse PUBLIC FILE_SET CXX_MODULES FILES src/parse.cppm)
target_link_libraries(cx_parse PUBLIC cx_core)

add_library(cx_spatial STATIC)
target_sources(cx_spatial PUBLIC FILE_SET CXX_MODULES FILES src/spatial.cppm)
target_link_libraries(cx_spatial PUBLIC cx_core cx_parse)

add_library(cx_render STATIC)
target_sources(cx_render PUBLIC FILE_SET CXX_MODULES FILES src/render.cppm)
target_link_libraries(cx_render PUBLIC cx_core cx_parse cx_spatial)

add_executable(cx-viz src/main.cpp)
target_link_libraries(cx-viz PRIVATE cx_core cx_parse cx_spatial cx_render)

foreach(tgt cx_core cx_parse cx_spatial cx_render cx-viz)
    target_compile_options(${tgt} PRIVATE -Wall -Wextra -O2)
endforeach()