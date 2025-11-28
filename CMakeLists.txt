cmake_minimum_required(VERSION 3.16)
project(Physics_Project LANGUAGES C CXX OBJC)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Optionally control deployment target / architectures here if desired
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0")
set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "Architectures to build for")

# --- Paths to vendored library sources ---
set(LIBRARY_SOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/library_sources")
set(SDL2_SOURCE_DIR "${LIBRARY_SOURCES_DIR}/SDL2")
set(SDL12COMPAT_SOURCE_DIR "${LIBRARY_SOURCES_DIR}/sdl12-compat")

# Ensure expected source directories exist
if(NOT EXISTS "${SDL2_SOURCE_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "SDL2 source not found at ${SDL2_SOURCE_DIR}. Expected SDL2 CMake project in library_sources/SDL2")
endif()
if(NOT EXISTS "${SDL12COMPAT_SOURCE_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "sdl12-compat source not found at ${SDL12COMPAT_SOURCE_DIR}. Expected sdl12-compat CMake project in library_sources/sdl12-compat")
endif()

# --- Build SDL2 from source (in-tree) ---
# Configure SDL2 to build as a static or shared library as desired.
# Here we build the default configuration and disable tests.
set(SDL_TEST OFF CACHE BOOL "Build SDL2 tests" FORCE)

add_subdirectory("${SDL2_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/SDL2-build")

# Try to detect the main SDL2 library target name
if(TARGET SDL2::SDL2)
    set(SDL2_MAIN_TARGET SDL2::SDL2)
elseif(TARGET SDL2::SDL2-static)
    set(SDL2_MAIN_TARGET SDL2::SDL2-static)
elseif(TARGET SDL2)
    set(SDL2_MAIN_TARGET SDL2)
else()
    message(FATAL_ERROR "Unable to find SDL2 CMake target after adding SDL2 subdirectory.")
endif()

# --- Build sdl12-compat from source (in-tree) using the in-tree SDL2 ---
# Hint sdl12-compat to use our SDL2 build by setting include and library hints.
get_target_property(SDL2_INCLUDE_DIRS ${SDL2_MAIN_TARGET} INTERFACE_INCLUDE_DIRECTORIES)

# If INTERFACE_INCLUDE_DIRECTORIES is not set, fall back to SDL2's source include dir
if(NOT SDL2_INCLUDE_DIRS)
    set(SDL2_INCLUDE_DIRS "${SDL2_SOURCE_DIR}/include")
endif()

set(SDL2_INCLUDE_DIR "${SDL2_INCLUDE_DIRS}" CACHE PATH "SDL2 include dir for sdl12-compat" FORCE)

add_subdirectory("${SDL12COMPAT_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/sdl12-compat-build")

# sdl12-compat's main library target is usually named SDL or SDL12_compat; detect it.
if(TARGET SDL)
    set(SDL12COMPAT_TARGET SDL)
elseif(TARGET SDL12_compat)
    set(SDL12COMPAT_TARGET SDL12_compat)
else()
    message(FATAL_ERROR "Unable to find sdl12-compat library target after adding sdl12-compat subdirectory.")
endif()

# --- Copy built libraries and headers into local include/SDL and lib ---
set(PROJECT_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib")
set(PROJECT_INCLUDE_SDL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include/SDL")

file(MAKE_DIRECTORY "${PROJECT_LIB_DIR}")
file(MAKE_DIRECTORY "${PROJECT_INCLUDE_SDL_DIR}")

# Custom target to copy the libraries and headers into the project tree.
# Use generator expressions to refer to the actual built library files.
add_custom_target(copy_sdl_libs_and_headers ALL
    COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:${SDL12COMPAT_TARGET}>" "${PROJECT_LIB_DIR}"
    COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:${SDL2_MAIN_TARGET}>" "${PROJECT_LIB_DIR}"
    # Copy only the contents of sdl12-compat/include/SDL into include/SDL,
    # so headers land as include/SDL/SDL.h, etc., not include/SDL/SDL/SDL.h
    COMMAND "${CMAKE_COMMAND}" -E copy_directory "${SDL12COMPAT_SOURCE_DIR}/include/SDL" "${PROJECT_INCLUDE_SDL_DIR}"
    COMMENT "Copying SDL2 and sdl12-compat libraries and headers into project include/SDL and lib"
)

# --- Main executable ---
add_executable(Physics_Project main.cpp)

# Ensure the copy target runs before the executable is linked
add_dependencies(Physics_Project copy_sdl_libs_and_headers)

# Use the SDL 1.2 compatibility headers we just copied into include/SDL
# so that `#include <SDL/SDL.h>` resolves correctly.
target_include_directories(Physics_Project PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Declare an imported shared library target for sdl12-compat that points to the
# copied library in lib/. This keeps the link logic simple and independent of
# the internal build location of sdl12-compat.

# On macOS, sdl12-compat builds libSDL-1.2.0.dylib by default.
set(SDL12COMPAT_LIB "${PROJECT_LIB_DIR}/libSDL-1.2.0.dylib")

add_library(SDL12_compat_imported SHARED IMPORTED GLOBAL)
set_target_properties(SDL12_compat_imported PROPERTIES
        IMPORTED_LOCATION "${SDL12COMPAT_LIB}"
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

# Ensure the executable can find the dylibs when run from the build tree.
set_target_properties(Physics_Project PROPERTIES
        BUILD_RPATH "${PROJECT_LIB_DIR}"
)

# On macOS, use SDL_main as the entry point if your code defines SDL_main.
if(APPLE)
    target_link_options(Physics_Project PRIVATE "-Wl,-e,_SDL_main")
endif()

# Link against the imported sdl12-compat library target.
target_link_libraries(Physics_Project PRIVATE SDL12_compat_imported)
