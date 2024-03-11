#pragma once
#include "../wayland/xdg-shell-client-protocol.h"
#include "../wayland/pointer-constraints-unstable-v1-protocol.h"
#include "../wayland/relative-pointer-unstable-v1.h"
#include "controls.hh"

#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <wayland-client-protocol.h>
#include <wayland-egl-core.h>
#include <string_view>

void drawFrame(void);
void setupDraw();

struct AppState
{
    int windowWidth;
    int windowHeight;

    bool programIsRunning;

    wl_surface* surface;
    xdg_surface* xdgSurface;
    xdg_toplevel* xdgToplevel;

    wl_egl_window *eglWindow;
    EGLDisplay eglDisplay;
    EGLContext eglContext;
    EGLSurface eglSurface;

    wl_seat *seat {};

    wl_pointer* pointer {};
    zwp_pointer_constraints_v1* pointerConstraints {};
    zwp_locked_pointer_v1* lockedPointer {};
    zwp_confined_pointer_v1* confinedPointer {};
    zwp_relative_pointer_v1* relativePointer {};
    /* lock/unlock pointer and switch to relative mode */
    zwp_relative_pointer_manager_v1* relativePointerManager {};

    std::string_view nameStr;

    bool paused = false;
    bool pointerLocked = false;

    void togglePointerRelativeMode();
};

extern const wl_callback_listener frameListener;

extern PlayerControls player;
extern AppState appState;
