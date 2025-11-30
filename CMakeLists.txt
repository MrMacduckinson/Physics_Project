cmake_minimum_required(VERSION 3.16)

# Option to build specifically against the macOS 10.15 SDK
option(BUILD_FOR_10_15 "Build for macOS 10.15 with custom SDK" ON)

# Configure SDK and architectures *before* project() so CMake tests use them
if(BUILD_FOR_10_15)
    set(MACOSX_10_15_SDK "/Library/Developer/CommandLineTools/SDKs/MacOSX10.15.sdk")
    if(NOT EXISTS "${MACOSX_10_15_SDK}")
        message(FATAL_ERROR "MacOSX10.15.sdk not found at ${MACOSX_10_15_SDK}")
    endif()

    set(CMAKE_OSX_SYSROOT "${MACOSX_10_15_SDK}" CACHE PATH "Sysroot for macOS 10.15" FORCE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.8" CACHE STRING "Deployment target" FORCE)
    # 10.15 CLT SDK is x86_64\-only
    set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "Architectures to build for (10.15 SDK is x86_64-only)" FORCE)
else()
    # Default build (modern macOS, still 10.15 min)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Deployment target" FORCE)
    set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "Architectures to build for" FORCE)
    # Let AppleClang pick the default sysroot (`/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk`)
endif()

project(Physics_Project LANGUAGES C CXX OBJC)

# Common C\+\+ settings for the non\-10\.15 build
if(NOT BUILD_FOR_10_15)
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

# Per\-configuration library/include/executable names
if(BUILD_FOR_10_15)
    set(PROJECT_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib_10_15")
    set(PROJECT_INCLUDE_SDL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include_10_15/SDL")
    set(EXECUTABLE_NAME "Physics_Project_10_15")
    set(INCLUDE_BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include_10_15")
else()
    set(PROJECT_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib")
    set(PROJECT_INCLUDE_SDL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include/SDL")
    set(EXECUTABLE_NAME "Physics_Project")
    set(INCLUDE_BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
endif()

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
set(SDL_TEST OFF CACHE BOOL "Build SDL2 tests" FORCE)
add_subdirectory("${SDL2_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/SDL2-build")

# Detect SDL2 main target
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
get_target_property(SDL2_INCLUDE_DIRS ${SDL2_MAIN_TARGET} INTERFACE_INCLUDE_DIRECTORIES)
if(NOT SDL2_INCLUDE_DIRS)
    set(SDL2_INCLUDE_DIRS "${SDL2_SOURCE_DIR}/include")
endif()
set(SDL2_INCLUDE_DIR "${SDL2_INCLUDE_DIRS}" CACHE PATH "SDL2 include dir for sdl12-compat" FORCE)

add_subdirectory("${SDL12COMPAT_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/sdl12-compat-build")

# Detect sdl12-compat target
if(TARGET SDL)
    set(SDL12COMPAT_TARGET SDL)
elseif(TARGET SDL12_compat)
    set(SDL12COMPAT_TARGET SDL12_compat)
else()
    message(FATAL_ERROR "Unable to find sdl12-compat library target after adding sdl12-compat subdirectory.")
endif()

# --- Copy built libraries and headers into local include/SDL and lib ---
file(MAKE_DIRECTORY "${PROJECT_LIB_DIR}")
file(MAKE_DIRECTORY "${PROJECT_INCLUDE_SDL_DIR}")

add_custom_target(copy_sdl_libs_and_headers ALL
        COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:${SDL12COMPAT_TARGET}>" "${PROJECT_LIB_DIR}"
        COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:${SDL2_MAIN_TARGET}>" "${PROJECT_LIB_DIR}"
        COMMAND "${CMAKE_COMMAND}" -E copy_directory "${SDL12COMPAT_SOURCE_DIR}/include/SDL" "${PROJECT_INCLUDE_SDL_DIR}"
        COMMENT "Copying SDL2 and sdl12-compat libraries and headers into project include/SDL and lib"
)

# --- Main executable ---
add_executable(${EXECUTABLE_NAME} main.c)
add_dependencies(${EXECUTABLE_NAME} copy_sdl_libs_and_headers)

target_include_directories(${EXECUTABLE_NAME} PRIVATE
        ${INCLUDE_BASE_DIR}
)

# Imported sdl12-compat from copied location
set(SDL12COMPAT_LIB "${PROJECT_LIB_DIR}/libSDL-1.2.0.dylib")

add_library(SDL12_compat_imported SHARED IMPORTED GLOBAL)
set_target_properties(SDL12_compat_imported PROPERTIES
        IMPORTED_LOCATION "${SDL12COMPAT_LIB}"
        INTERFACE_INCLUDE_DIRECTORIES "${INCLUDE_BASE_DIR}"
)

# Ensure the executable can find the dylibs when run from the build tree.
set_target_properties(${EXECUTABLE_NAME} PROPERTIES
        BUILD_RPATH "${PROJECT_LIB_DIR}"
)

# macOS\-specific link options
if(APPLE)
    if(BUILD_FOR_10_15)
        # Extra safety: force sysroot and min version for the main target
        target_compile_options(${EXECUTABLE_NAME} PRIVATE
                "-isysroot" "${CMAKE_OSX_SYSROOT}"
                "-mmacosx-version-min=10.8"
        )
        target_link_options(${EXECUTABLE_NAME} PRIVATE
                "-isysroot" "${CMAKE_OSX_SYSROOT}"
                "-mmacosx-version-min=10.8"
        )
    endif()
    target_link_options(${EXECUTABLE_NAME} PRIVATE "-Wl,-e,_SDL_main")
endif()

target_link_libraries(${EXECUTABLE_NAME} PRIVATE SDL12_compat_imported)
