cmake_minimum_required(VERSION 3.8)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 20)
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
    src/shader.cc
    src/texture.cc
    src/rng.cc
    src/json/lex.cc
    src/json/parser.cc
    src/gltf/gltf.cc
    src/gl/gl.cc
    src/parser/bin.cc
    src/parser/obj.cc
)

if (FPS_COUNTER)
    add_definitions("-DFPS_COUNTER")
endif()
if (LOGS)
    add_definitions("-DLOGS")
endif()
if (MODEL)
    add_definitions("-DMODEL")
endif()

if (CMAKE_BUILD_TYPE MATCHES "Asan")
    set(CMAKE_BUILD_TYPE "Debug")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g3 -fsanitize=undefined -fsanitize=address")
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

message(STATUS "CMAKE_SYSTEM_NAME: '${CMAKE_SYSTEM_NAME}'")
message(STATUS "CMAKE_SOURCE_DIR: '${CMAKE_SOURCE_DIR}'")

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LINUX_PKGS REQUIRED gl egl glesv2 wayland-client wayland-egl wayland-cursor)
    target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${LINUX_PKGS_INCLUDE_DIRS})
    target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${LINUX_PKGS_LIBRARIES})

    set(WLP "./src/platform/wayland")
    set(WLPD "${WLP}/wayland-protocols")
    execute_process(
        COMMAND pkg-config wayland-protocols --variable=pkgdatadir
        OUTPUT_VARIABLE WAYLAND_PROTOCOLS_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
        COMMAND pkg-config --variable=wayland_scanner wayland-scanner
        OUTPUT_VARIABLE WAYLAND_SCANNER
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(XDG_SHELL "${WAYLAND_PROTOCOLS_DIR}/stable/xdg-shell/xdg-shell.xml")
    set(POINTER_CONSTRAINTS "${WAYLAND_PROTOCOLS_DIR}/unstable/pointer-constraints/pointer-constraints-unstable-v1.xml")
    set(RELATIVE_POINTER "${WAYLAND_PROTOCOLS_DIR}/unstable/relative-pointer/relative-pointer-unstable-v1.xml")

    message(STATUS "WLP: '${WLP}'")
    message(STATUS "WLPD: '${WLPD}'")
    message(STATUS "WAYLAND_PROTOCOLS_DIR: '${WAYLAND_PROTOCOLS_DIR}'")
    message(STATUS "WAYLAND_SCANNER: '${WAYLAND_SCANNER}'")
    message(STATUS "XDG_SHELL: '${XDG_SHELL}'")
    message(STATUS "POINTER_CONSTRAINTS: '${POINTER_CONSTRAINTS}'")
    message(STATUS "RELATIVE_POINTER: '${RELATIVE_POINTER}'")

    execute_process(COMMAND mkdir -p ${CMAKE_SOURCE_DIR}/${WLPD})
    execute_process(COMMAND ${WAYLAND_SCANNER} client-header ${RELATIVE_POINTER}    ${CMAKE_SOURCE_DIR}/${WLPD}/relative-pointer-unstable-v1.h)
    execute_process(COMMAND ${WAYLAND_SCANNER} private-code  ${RELATIVE_POINTER}    ${CMAKE_SOURCE_DIR}/${WLPD}/relative-pointer-unstable-v1.c)
    execute_process(COMMAND ${WAYLAND_SCANNER} client-header ${POINTER_CONSTRAINTS} ${CMAKE_SOURCE_DIR}/${WLPD}/pointer-constraints-unstable-v1.h)
    execute_process(COMMAND ${WAYLAND_SCANNER} private-code  ${POINTER_CONSTRAINTS} ${CMAKE_SOURCE_DIR}/${WLPD}/pointer-constraints-unstable-v1.c)
    execute_process(COMMAND ${WAYLAND_SCANNER} client-header ${XDG_SHELL}           ${CMAKE_SOURCE_DIR}/${WLPD}/xdg-shell.h)
    execute_process(COMMAND ${WAYLAND_SCANNER} private-code  ${XDG_SHELL}           ${CMAKE_SOURCE_DIR}/${WLPD}/xdg-shell.c)

    target_sources(
        ${CMAKE_PROJECT_NAME}
        PRIVATE
        ${WLP}/input.cc
        ${WLP}/wayland.cc
        ${WLP}/input.cc
        ${WLPD}/pointer-constraints-unstable-v1.c
        ${WLPD}/relative-pointer-unstable-v1.c
        ${WLPD}/xdg-shell.c
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
