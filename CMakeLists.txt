cmake_minimum_required(VERSION 3.16)

# Build options
option(BUILD_FOR_10_15 "Build for macOS 10.15 with custom SDK" OFF)
option(BUILD_FOR_10_4 "Build for macOS 10.4-10.6 Intel with SDL 1.2" OFF)

# Configure SDK and architectures *before* project()
if(BUILD_FOR_10_4)
    set(MACOSX_10_4_SDK "/Library/Developer/CommandLineTools/SDKs/MacOSX10.4u.sdk")
    if(NOT EXISTS "${MACOSX_10_4_SDK}")
        message(FATAL_ERROR "MacOSX10.4u.sdk not found at ${MACOSX_10_4_SDK}")
    endif()

    set(CMAKE_OSX_SYSROOT "${MACOSX_10_4_SDK}" CACHE PATH "Sysroot for macOS 10.4" FORCE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.4" CACHE STRING "Deployment target" FORCE)
    set(CMAKE_OSX_ARCHITECTURES "i386" CACHE STRING "Architectures to build for (10.4 SDK Intel)" FORCE)

    # SDL 1.2 build configuration
    set(SDL12_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/library_sources/SDL-1.2")
    set(SDL12_BUILD_DIR "${CMAKE_BINARY_DIR}/SDL-1.2-build")
    set(SDL12_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/SDL-1.2-install")

    if(NOT EXISTS "${SDL12_SOURCE_DIR}/configure")
        message(FATAL_ERROR "SDL 1.2 source not found at ${SDL12_SOURCE_DIR}")
    endif()

elseif(BUILD_FOR_10_15)
    set(MACOSX_10_15_SDK "/Library/Developer/CommandLineTools/SDKs/MacOSX10.15.sdk")
    if(NOT EXISTS "${MACOSX_10_15_SDK}")
        message(FATAL_ERROR "MacOSX10.15.sdk not found at ${MACOSX_10_15_SDK}")
    endif()

    set(CMAKE_OSX_SYSROOT "${MACOSX_10_15_SDK}" CACHE PATH "Sysroot for macOS 10.15" FORCE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.7" CACHE STRING "Deployment target" FORCE)
    set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "Architectures to build for (10.15 SDK is x86_64-only)" FORCE)
else()
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Deployment target" FORCE)
    set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "Architectures to build for" FORCE)
endif()

project(Physics_Project LANGUAGES C OBJC)

if(NOT BUILD_FOR_10_15 AND NOT BUILD_FOR_10_4)
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

# Per-configuration settings
if(BUILD_FOR_10_4)
    set(PROJECT_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib_10_4")
    set(PROJECT_INCLUDE_SDL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include_10_4/SDL")
    set(EXECUTABLE_NAME "Physics_Project_10_4")
    set(INCLUDE_BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include_10_4")

    # Build SDL 1.2 using ExternalProject
    include(ExternalProject)
    ExternalProject_Add(SDL12_build
            SOURCE_DIR "${SDL12_SOURCE_DIR}"
            BINARY_DIR "${SDL12_BUILD_DIR}"
            CONFIGURE_COMMAND ${SDL12_SOURCE_DIR}/configure
            --prefix=${SDL12_INSTALL_PREFIX}
            --host=i386-apple-darwin
            --disable-shared
            --enable-static
            CC=${CMAKE_C_COMPILER}
            CFLAGS=-isysroot\ ${CMAKE_OSX_SYSROOT}\ -mmacosx-version-min=10.4\ -arch\ i386\ -include\ stddef.h
            LDFLAGS=-isysroot\ ${CMAKE_OSX_SYSROOT}\ -mmacosx-version-min=10.4\ -arch\ i386
            BUILD_COMMAND make
            INSTALL_COMMAND make install
            BUILD_ALWAYS OFF
    )

    set(SDL12_STATIC_LIB "${SDL12_INSTALL_PREFIX}/lib/libSDL.a")
    set(SDL12_INCLUDE_DIR "${SDL12_INSTALL_PREFIX}/include/SDL")

    # Create directories to satisfy CMake's imported target validation
    file(MAKE_DIRECTORY "${SDL12_INSTALL_PREFIX}/lib")
    file(MAKE_DIRECTORY "${SDL12_INSTALL_PREFIX}/include")


elseif(BUILD_FOR_10_15)
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

# --- SDL2/sdl12-compat build (for non-10.4 builds) ---
if(NOT BUILD_FOR_10_4)
    set(LIBRARY_SOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/library_sources")
    set(SDL2_SOURCE_DIR "${LIBRARY_SOURCES_DIR}/SDL2")
    set(SDL12COMPAT_SOURCE_DIR "${LIBRARY_SOURCES_DIR}/sdl12-compat")

    if(NOT EXISTS "${SDL2_SOURCE_DIR}/CMakeLists.txt")
        message(FATAL_ERROR "SDL2 source not found at ${SDL2_SOURCE_DIR}")
    endif()
    if(NOT EXISTS "${SDL12COMPAT_SOURCE_DIR}/CMakeLists.txt")
        message(FATAL_ERROR "sdl12-compat source not found at ${SDL12COMPAT_SOURCE_DIR}")
    endif()

    set(SDL_TEST OFF CACHE BOOL "Build SDL2 tests" FORCE)
    add_subdirectory("${SDL2_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/SDL2-build")

    if(TARGET SDL2::SDL2)
        set(SDL2_MAIN_TARGET SDL2::SDL2)
    elseif(TARGET SDL2::SDL2-static)
        set(SDL2_MAIN_TARGET SDL2::SDL2-static)
    elseif(TARGET SDL2)
        set(SDL2_MAIN_TARGET SDL2)
    else()
        message(FATAL_ERROR "Unable to find SDL2 CMake target")
    endif()

    get_target_property(SDL2_INCLUDE_DIRS ${SDL2_MAIN_TARGET} INTERFACE_INCLUDE_DIRECTORIES)
    if(NOT SDL2_INCLUDE_DIRS)
        set(SDL2_INCLUDE_DIRS "${SDL2_SOURCE_DIR}/include")
    endif()
    set(SDL2_INCLUDE_DIR "${SDL2_INCLUDE_DIRS}" CACHE PATH "SDL2 include dir for sdl12-compat" FORCE)

    add_subdirectory("${SDL12COMPAT_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/sdl12-compat-build")

    if(TARGET SDL)
        set(SDL12COMPAT_TARGET SDL)
    elseif(TARGET SDL12_compat)
        set(SDL12COMPAT_TARGET SDL12_compat)
    else()
        message(FATAL_ERROR "Unable to find sdl12-compat library target")
    endif()

    file(MAKE_DIRECTORY "${PROJECT_LIB_DIR}")
    file(MAKE_DIRECTORY "${PROJECT_INCLUDE_SDL_DIR}")

    add_custom_target(copy_sdl_libs_and_headers ALL
            COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:${SDL12COMPAT_TARGET}>" "${PROJECT_LIB_DIR}"
            COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:${SDL2_MAIN_TARGET}>" "${PROJECT_LIB_DIR}"
            COMMAND "${CMAKE_COMMAND}" -E copy_directory "${SDL12COMPAT_SOURCE_DIR}/include/SDL" "${PROJECT_INCLUDE_SDL_DIR}"
            COMMENT "Copying SDL2 and sdl12-compat libraries and headers"
    )
endif()

# --- Main executable ---
if(BUILD_FOR_10_4)
    add_executable(${EXECUTABLE_NAME} main.c eprintf_stub.c)
    set_source_files_properties(main.c PROPERTIES LANGUAGE OBJC)
    target_compile_definitions(${EXECUTABLE_NAME} PRIVATE BUILD_FOR_10_4)
else()
    add_executable(${EXECUTABLE_NAME} main.c)
endif()

target_include_directories(${EXECUTABLE_NAME} PRIVATE ${INCLUDE_BASE_DIR})

if(BUILD_FOR_10_4)
    # SDL 1.2 static linking
    add_dependencies(${EXECUTABLE_NAME} SDL12_build)

    add_library(SDL12_static STATIC IMPORTED GLOBAL)
    set_target_properties(SDL12_static PROPERTIES
            IMPORTED_LOCATION "${SDL12_STATIC_LIB}"
            INTERFACE_INCLUDE_DIRECTORIES "${SDL12_INSTALL_PREFIX}/include"
    )

    target_link_libraries(${EXECUTABLE_NAME} PRIVATE
            SDL12_static
            "-framework Cocoa"
            "-framework Carbon"
            "-framework IOKit"
            "-framework CoreAudio"
            "-framework AudioToolbox"
            "-framework AudioUnit"
            "-framework OpenGL"
            "-framework ApplicationServices"
    )

    target_compile_options(${EXECUTABLE_NAME} PRIVATE
            "-isysroot" "${CMAKE_OSX_SYSROOT}"
            "-mmacosx-version-min=10.4"
    )
    target_link_options(${EXECUTABLE_NAME} PRIVATE
            "-isysroot" "${CMAKE_OSX_SYSROOT}"
            "-mmacosx-version-min=10.4"
    )
else()
    add_dependencies(${EXECUTABLE_NAME} copy_sdl_libs_and_headers)

    set(SDL12COMPAT_LIB "${PROJECT_LIB_DIR}/libSDL-1.2.0.dylib")
    add_library(SDL12_compat_imported SHARED IMPORTED GLOBAL)
    set_target_properties(SDL12_compat_imported PROPERTIES
            IMPORTED_LOCATION "${SDL12COMPAT_LIB}"
            INTERFACE_INCLUDE_DIRECTORIES "${INCLUDE_BASE_DIR}"
    )

    set_target_properties(${EXECUTABLE_NAME} PROPERTIES BUILD_RPATH "${PROJECT_LIB_DIR}")

    if(APPLE AND BUILD_FOR_10_15)
        target_compile_options(${EXECUTABLE_NAME} PRIVATE
                "-isysroot" "${CMAKE_OSX_SYSROOT}"
                "-mmacosx-version-min=10.7"
        )
        target_link_options(${EXECUTABLE_NAME} PRIVATE
                "-isysroot" "${CMAKE_OSX_SYSROOT}"
                "-mmacosx-version-min=10.7"
        )
    endif()

    target_link_libraries(${EXECUTABLE_NAME} PRIVATE SDL12_compat_imported)
endif()
