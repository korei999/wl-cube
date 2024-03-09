#define _POSIX_C_SOURCE 199309L
#include "headers/main.hh"
#include "headers/utils.hh"
#include "headers/input.hh"

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl32.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include <wayland-egl.h>
#ifdef __linux__
#    include <linux/input-event-codes.h>
#elif __FreeBSD__
#    include <dev/evdev/input-event-codes.h>
#endif

#include "glueCode/xdg-shell-client-protocol.h"

int windowWidth = 600;
int windowHeight = 420;

bool programIsRunning = true;

static struct wl_compositor* compositor = nullptr;
static struct xdg_wm_base* xdg_wm_base = nullptr;

struct wl_surface* surface = nullptr;
struct xdg_surface* xdg_surface = nullptr;
struct xdg_toplevel* xdg_toplevel = nullptr;

struct wl_egl_window* egl_window = nullptr;
EGLDisplay egl_display = nullptr;
EGLContext egl_context = nullptr;
EGLSurface egl_surface = nullptr;

uint32_t xdg_configure_serial = 0;

static void
frame_handle_done(void* data, struct wl_callback* callback, uint32_t time)
{
    wl_callback_destroy(callback);
    drawFrame();
}

const struct wl_callback_listener frame_listener = {
    .done = frame_handle_done,
};

static void
xdg_surface_handle_configure(void* data, struct xdg_surface* xdg_surface, uint32_t serial)
{
    xdg_surface_ack_configure(xdg_surface, serial);
}

static const struct xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_handle_configure,
};

static void
configure_handle(void* data, struct xdg_toplevel* xdg_toplevel, int32_t width, int32_t height, struct wl_array* states)
{
    if (width != windowWidth || height != windowHeight)
    {
        wl_egl_window_resize(egl_window, width, height, 0, 0);
        windowWidth = width;
        windowHeight = height;

        D(glViewport(0, 0, width, height));
    }
}

static void
xdg_toplevel_handle_close(void* data, struct xdg_toplevel* xdg_toplevel)
{
    programIsRunning = false;
}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
    .configure = configure_handle,
    .close = xdg_toplevel_handle_close,
};

static const wl_pointer_listener pointerListener {
    .enter = pointerEnterHandle,
    .leave = pointerLeaveHandle,
    .motion = pointerMotionHandle,
    .button = pointerButtonHandle,
    .axis = pointerAxisHandle,
};

static const wl_keyboard_listener keyboardListener {
    .keymap = keyboardKeymapHandle,
    .enter = keyboardEnterHandle,
    .leave = keyboardLeaveHandle,
    .key = keyboardKeyHandle,
    .modifiers = keyboardModifiersHandle,
    .repeat_info = keyboardRepeatInfo
};

static void
seatHandleCapabilities(void* data, struct wl_seat* seat, uint32_t capabilities)
{
    if (capabilities & WL_SEAT_CAPABILITY_POINTER)
    {
        wl_pointer* pointer = wl_seat_get_pointer(seat);
        wl_pointer_add_listener(pointer, &pointerListener, seat);
        LOG(GOOD, "pointer works.\n");
    }
    if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD)
    {
        wl_keyboard* keyboard = wl_seat_get_keyboard(seat);
        wl_keyboard_add_listener(keyboard, &keyboardListener, seat);
        LOG(GOOD, "keyboard works.\n");
    }
    if (capabilities & WL_SEAT_CAPABILITY_TOUCH)
    {
        //
        LOG(GOOD, "touch works.\n");
    }
}

static const struct wl_seat_listener seat_listener = {
    .capabilities = seatHandleCapabilities,
};

static void
handle_global(void* data, struct wl_registry* registry, uint32_t name, const char* interface, uint32_t version)
{
    if (strcmp(interface, wl_seat_interface.name) == 0)
    {
        struct wl_seat* seat = (struct wl_seat*)wl_registry_bind(registry, name, &wl_seat_interface, 1);
        wl_seat_add_listener(seat, &seat_listener, nullptr);
    }
    else if (strcmp(interface, wl_compositor_interface.name) == 0)
    {
        compositor = (struct wl_compositor*)wl_registry_bind(registry, name, &wl_compositor_interface, 1);
    }
    else if (strcmp(interface, xdg_wm_base_interface.name) == 0)
    {
        xdg_wm_base = (struct xdg_wm_base*)wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
    }
}

static void
handle_global_remove(void* data, struct wl_registry* registry, uint32_t name)
{
    // Who cares
}

static const struct wl_registry_listener registry_listener = {
    .global = handle_global,
    .global_remove = handle_global_remove,
};

int
main(int argc, char* argv[])
{
    struct wl_display* display = wl_display_connect(nullptr);
    if (display == nullptr)
    {
        fprintf(stderr, "failed to create display\n");
        return EXIT_FAILURE;
    }

    struct wl_registry* registry = wl_display_get_registry(display);
    wl_registry_add_listener(registry, &registry_listener, nullptr);
    wl_display_dispatch(display);
    wl_display_roundtrip(display);

    if (compositor == nullptr || xdg_wm_base == nullptr)
    {
        fprintf(stderr, "no wl_shm, wl_compositor or xdg_wm_base support\n");
        return EXIT_FAILURE;
    }

    egl_display = eglGetDisplay((EGLNativeDisplayType)display);
    EGLD();
    if (egl_display == EGL_NO_DISPLAY)
    {
        fprintf(stderr, "failed to create EGL display\n");
        return EXIT_FAILURE;
    }

    EGLint major, minor;
    if (!eglInitialize(egl_display, &major, &minor))
    {
        fprintf(stderr, "failed to initialize EGL\n");
        return EXIT_FAILURE;
    }
    EGLD();
    LOG(OK, "egl: major: {}, minor: {}\n", major, minor);

    EGLint count;
    EGLD(eglGetConfigs(egl_display, nullptr, 0, &count));
    LOG(OK, "count: {}\n", count);

    EGLint config_attribs[] = {
        EGL_SURFACE_TYPE,
        EGL_WINDOW_BIT,
        EGL_RED_SIZE,
        8,
        EGL_GREEN_SIZE,
        8,
        EGL_BLUE_SIZE,
        6,
        EGL_DEPTH_SIZE,
        24,
        EGL_RENDERABLE_TYPE,
        EGL_OPENGL_ES3_BIT_KHR,
        EGL_MIN_SWAP_INTERVAL,
        0,
        EGL_NONE,
    };
    EGLint n = 0;
    EGLConfig* configs = new EGLConfig[count] {};
    EGLD(eglChooseConfig(egl_display, config_attribs, configs, count, &n));
    if (n == 0)
    {
        LOG(FATAL, "Failed to choose an EGL config\n");
    }
    EGLConfig egl_config = configs[0];

    EGLint context_attribs[] = {
        EGL_CONTEXT_CLIENT_VERSION,
        3,
        EGL_NONE,
    };
    egl_context = eglCreateContext(egl_display, egl_config, EGL_NO_CONTEXT, context_attribs);
    EGLD();

    surface = wl_compositor_create_surface(compositor);
    xdg_surface = xdg_wm_base_get_xdg_surface(xdg_wm_base, surface);
    xdg_toplevel = xdg_surface_get_toplevel(xdg_surface);

    xdg_toplevel_set_title(xdg_toplevel, "wl-cube");
    xdg_toplevel_set_app_id(xdg_toplevel, "wl-cube");

    xdg_surface_add_listener(xdg_surface, &xdg_surface_listener, nullptr);
    xdg_toplevel_add_listener(xdg_toplevel, &xdg_toplevel_listener, nullptr);

    egl_window = wl_egl_window_create(surface, windowWidth, windowHeight);
    EGLD(egl_surface = eglCreateWindowSurface(egl_display, egl_config, (EGLNativeWindowType)egl_window, nullptr));

    wl_surface_commit(surface);
    wl_display_roundtrip(display);

    // Draw the first frame
    setupDraw();
    drawFrame();

    while (wl_display_dispatch(display) != -1 && programIsRunning)
    {
        // This space intentionally left blank
    }

    fprintf(stderr, "cleanup ...\n");
    xdg_toplevel_destroy(xdg_toplevel);
    xdg_surface_destroy(xdg_surface);
    wl_surface_destroy(surface);

    wl_egl_window_destroy(egl_window);
    EGLD(eglDestroySurface(egl_display, egl_surface));
    EGLD(eglDestroyContext(egl_display, egl_context));

    delete[] configs;

    return EXIT_SUCCESS;
}
