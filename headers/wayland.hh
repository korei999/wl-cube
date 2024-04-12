#pragma once
#include "../wayland-protocols/xdg-shell.h"
#include "../wayland-protocols/pointer-constraints-unstable-v1.h"
#include "../wayland-protocols/relative-pointer-unstable-v1.h"
#include "ultratypes.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <wayland-client-protocol.h>
#include <wayland-cursor.h>
#include <wayland-egl-core.h>
#include <string_view>

struct WlClient
{
    int wWidth = 1920;
    int wHeight = 1080;

    bool isRunning = false;
    bool isConfigured = false;
    int swapInterval = 1;

    bool isPaused = false;
    bool isRelativeMode = false;
    bool isFullscreen = false;

    std::string_view appName = "wl-cube";

    wl_display* display {};
    wl_registry* registry {};

    wl_surface* surface {};
    xdg_surface* xdgSurface {};
    xdg_toplevel* xdgToplevel {};
    wl_output* output {};

    wl_egl_window *eglWindow {};
    EGLDisplay eglDisplay {};
    EGLContext eglContext {};
    EGLSurface eglSurface {};

    wl_seat* seat {};
    wl_shm* shm {};
    wl_compositor* compositor {};
    xdg_wm_base* xdgWmBase {};
    [[maybe_unused]] u32 xdgConfigureSerial = 0;

    wl_pointer* pointer {};
    wl_surface* cursorSurface {};
    wl_cursor_image* cursorImage {};
    wl_cursor_theme* cursorTheme {};

    u32 pointerSerial = 0;
    zwp_pointer_constraints_v1* pointerConstraints {};
    zwp_locked_pointer_v1* lockedPointer {};
    zwp_confined_pointer_v1* confinedPointer {};
    zwp_relative_pointer_v1* relativePointer {};
    zwp_relative_pointer_manager_v1* relativePointerManager {};

    wl_keyboard* keyboard {};

    ~WlClient();

    void init();
    void mainLoop();
    void disableRelativeMode();
    void enableRelativeMode();
    void togglePointerRelativeMode();
    void toggleFullscreen();
    void setCursor(std::string_view cursorType);
    void setFullscreen();
    void unsetFullscreen();
    void drawFrame();
    void setupDraw();
    void bindGlContext();
    void unbindGlContext();
    void setSwapInterval(int interval);
};
