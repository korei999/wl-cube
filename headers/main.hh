#pragma once
#include "../glueCode/xdg-shell-client-protocol.h"
#include "controls.hh"

#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <wayland-client-protocol.h>
#include <wayland-egl-core.h>
#include <string_view>

void drawFrame(void);
void setupDraw();


struct AppState {
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

    std::string_view nameStr;

    bool paused = false;
};

extern const wl_callback_listener frameListener;

extern PlayerControls player;
extern AppState appState;
