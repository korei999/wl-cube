cmake_minimum_required(VERSION 3.8)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

project(wl-cube VERSION 0.2 LANGUAGES CXX C)
message(STATUS "CMAKE_PROJECT_VERSION: '${CMAKE_PROJECT_VERSION}'")

add_compile_options(-Wall -Wextra -fms-extensions)
include_directories(BEFORE "utils")

add_executable(
    ${CMAKE_PROJECT_NAME}
    src/main.cc
    src/model.cc
    src/controls.cc
    src/frame.cc
    src/gmath.cc
    src/parser.cc
    src/shader.cc
    src/texture.cc
    src/rng.cc
    src/json/lex.cc
    src/json/parser.cc
    src/gltf/gltf.cc
    src/gl/gl.cc
)

if (FPS_COUNTER)
    add_definitions("-DFPS_COUNTER")
endif()

if (LOGS)
    add_definitions("-DLOGS")
endif()

if (CMAKE_BUILD_TYPE MATCHES "Asan")
    set(CMAKE_BUILD_TYPE "Debug")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g3 -fsanitize=undefined -fsanitize=address ")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g3 -fsanitize=undefined -fsanitize=address")
    add_link_options(-fsanitize=undefined -fsanitize=address)
endif()

message(STATUS "CMAKE_BUILD_TYPE: '${CMAKE_BUILD_TYPE}'")

if (CMAKE_BUILD_TYPE MATCHES "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wmissing-field-initializers")
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function)
    add_definitions("-DLOGS") # used for LOG_* macros
    add_definitions("-DDEBUG")
    add_definitions("-DFPS_COUNTER")
endif()

set_property(TARGET ${CMAKE_PROJECT_NAME} PROPERTY CXX_STANDARD 20)

message(STATUS "CMAKE_SYSTEM_NAME: '${CMAKE_SYSTEM_NAME}'")

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LINUX_PKGS REQUIRED gl egl glesv2 wayland-client wayland-egl wayland-cursor)
    target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${LINUX_PKGS_INCLUDE_DIRS})
    target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${LINUX_PKGS_LIBRARIES})

    target_sources(
        ${CMAKE_PROJECT_NAME}
        PRIVATE
        src/platform/wayland/input.cc
        src/platform/wayland/wayland.cc
        src/platform/wayland/input.cc
        src/platform/wayland/wayland-protocols/pointer-constraints-unstable-v1.c
        src/platform/wayland/wayland-protocols/relative-pointer-unstable-v1.c
        src/platform/wayland/wayland-protocols/xdg-shell.c
    )
elseif (CMAKE_SYSTEM_NAME MATCHES "Windows")
    find_package(OpenGL REQUIRED)
    message(STATUS "OpenGL: '${OpenGL}'")
    target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${OPENGL_gl_LIBRARY})

    target_sources(
        ${CMAKE_PROJECT_NAME}
        PRIVATE
        src/platform/windows/glad.cc
        src/platform/windows/input.cc
        src/platform/windows/windows.cc
    )
endif()

# install(TARGETS ${CMAKE_PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
