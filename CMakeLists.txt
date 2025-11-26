cmake_minimum_required(VERSION 3.16)
project(Physics_Project)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# Main executable
add_executable(Physics_Project main.cpp)

# Use the SDL 1.2 compatibility headers shipped with the project.
# These come from the sdl12-compat repo and should live under:
#   include/SDL/SDL.h, etc.
# This lets code with `#include <SDL/SDL.h>` build without any
# system-wide SDL installation.
target_include_directories(Physics_Project PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Path to the prebuilt sdl12-compat library shipped with the project.
# On macOS, the default name from sdl12-compat is usually libSDL-1.2.0.dylib.
# Adjust the filename here if your actual library file is different.
set(SDL12COMPAT_LIB "${CMAKE_CURRENT_SOURCE_DIR}/lib/libSDL-1.2.0.dylib")

if(NOT EXISTS "${SDL12COMPAT_LIB}")
    message(FATAL_ERROR "sdl12-compat library not found at ${SDL12COMPAT_LIB}.\n"
                        "Make sure you have built sdl12-compat and copied the resulting dylib into the lib/ directory.")
endif()

# Declare an imported shared library target for sdl12-compat so that
# we can link to it in a CMake-idiomatic way and propagate include dirs.
add_library(SDL12_compat SHARED IMPORTED GLOBAL)
set_target_properties(SDL12_compat PROPERTIES
    IMPORTED_LOCATION "${SDL12COMPAT_LIB}"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

# Ensure the executable can find the dylib when run from the build tree.
# BUILD_RPATH is used by CMake to embed an rpath pointing to our local lib/ dir.
set_target_properties(Physics_Project PROPERTIES
    BUILD_RPATH "${CMAKE_CURRENT_SOURCE_DIR}/lib"
)

if(APPLE)
    target_link_options(Physics_Project PRIVATE "-Wl,-e,_SDL_main")
endif()
# Link against the sdl12-compat library target.
# This pulls in the prebuilt dylib and the compatibility headers.
target_link_libraries(Physics_Project PRIVATE SDL12_compat)
