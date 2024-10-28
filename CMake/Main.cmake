# Global configuration types
set(CMAKE_CONFIGURATION_TYPES
        "Debug"
        "Release")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${EXECUTABLE_OUTPUT_PATH}/$<0:>")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${EXECUTABLE_OUTPUT_PATH}/$<0:>")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${LIBRARY_OUTPUT_PATH}/$<0:>")

include(CMake/CompilerOptions.cmake)
include(CMake/CompilerDefinitions.cmake)
include(CMake/Functions.cmake)
