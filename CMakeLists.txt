cmake_minimum_required(VERSION 3.27)

project(Sample VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CMake/Main.cmake)

add_subdirectory(WfpExample)
