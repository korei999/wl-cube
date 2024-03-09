#pragma once
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <wayland-egl-core.h>

void drawFrame(void);
void setupDraw();

extern int windowWidth;
extern int windowHeight;

extern bool programIsRunning;

extern struct wl_surface* surface;
extern struct xdg_surface* xdg_surface;
extern struct xdg_toplevel* xdg_toplevel;

extern wl_egl_window *egl_window;
extern EGLDisplay egl_display;
extern EGLContext egl_context;
extern EGLSurface egl_surface;

extern uint32_t xdg_configure_serial;

extern const struct wl_callback_listener frame_listener;
