#pragma once
#include "../wayland/xdg-shell.h"
#include "../wayland/pointer-constraints-unstable-v1.h"
#include "../wayland/relative-pointer-unstable-v1.h"
#include "ultratypes.h"
#include "utils.hh"

#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <string_view>
#include <wayland-client-protocol.h>
#include <wayland-cursor.h>
#include <wayland-egl-core.h>

void drawFrame();
void setupDraw();
void swapFrames();

struct AppState
{
    int wWidth {};
    int wHeight {};

    bool programIsRunning = false;

    wl_display* display {};

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

    std::string_view nameStr {};

    bool isPaused = false;
    bool isRelativeMode = false;
    bool isFullscreen = false;

    ~AppState()
    {
        LOG(OK, "cleanup ...\n");
        xdg_toplevel_destroy(xdgToplevel);
        xdg_surface_destroy(xdgSurface);
        wl_surface_destroy(surface);
        wl_cursor_theme_destroy(cursorTheme);

        wl_egl_window_destroy(eglWindow);
        EGLD(eglDestroySurface(eglDisplay, eglSurface));
        EGLD(eglDestroyContext(eglDisplay, eglContext));
    }

    void
    togglePointerRelativeMode()
    {
        isRelativeMode = !isRelativeMode;

        if (isRelativeMode)
            enableRelativeMode();
        else
            disableRelativeMode();

        LOG(OK, "relative mode: {}\n", isRelativeMode);
    }

    void
    toggleFullscreen()
    {
        isFullscreen = !isFullscreen;
        if (isFullscreen)
            setFullscreen();
        else
            unsetFullscreen();
    }

    void setCursor(std::string_view cursorType = "left_ptr");
    void enableRelativeMode();
    void disableRelativeMode();

private:
    void setFullscreen();
    void unsetFullscreen();
};

extern const wl_callback_listener frameListener;
extern AppState appState;
