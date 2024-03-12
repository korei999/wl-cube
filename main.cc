#define _POSIX_C_SOURCE 199309L
#include "headers/main.hh"
#include "headers/utils.hh"
#include "headers/input.hh"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <vector>

AppState appState {
    .windowWidth = 640,
    .windowHeight = 480,

    .programIsRunning = true,

    .surface = nullptr,
    .xdgSurface = nullptr,
    .xdgToplevel = nullptr,

    .eglWindow = nullptr,
    .eglDisplay = nullptr,
    .eglContext = nullptr,
    .eglSurface = nullptr,

    .nameStr {}
};

enum
{
	REGION_TYPE_NONE,
	REGION_TYPE_DISJOINT,
	REGION_TYPE_JOINT,
	REGION_TYPE_MAX
} region_type = REGION_TYPE_NONE;

static wl_compositor* compositor = nullptr;
static xdg_wm_base* xdgWmBase = nullptr;
static u32 xdgConfigureSerial = 0;

static const zwp_relative_pointer_v1_listener relativePointerListener = {
	.relative_motion = relativePointerHandleMotion
};

void
AppState::togglePointerRelativeMode()
{
    if (!pointerRelativeMode)
    {
        wl_pointer_set_cursor(appState.pointer, appState.pointerSerial, NULL, 0, 0);
        pointerRelativeMode = true;
        lockedPointer = zwp_pointer_constraints_v1_lock_pointer(pointerConstraints,
                                                                surface,
                                                                pointer,
                                                                nullptr,
                                                                ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_ONESHOT);
        // zwp_locked_pointer_v1_set_cursor_position_hint(lockedPointer, wl_fixed_from_int(-1), wl_fixed_from_int(-1));
        relativePointer = zwp_relative_pointer_manager_v1_get_relative_pointer(relativePointerManager, pointer);
        zwp_relative_pointer_v1_add_listener(relativePointer, &relativePointerListener, nullptr);

    }
    else
    {
        // wl_pointer_set_cursor(appState.pointer, appState.pointerSerial, NULL, 0, 0);
        pointerRelativeMode = false;
        zwp_locked_pointer_v1_destroy(lockedPointer);
        zwp_relative_pointer_v1_destroy(relativePointer);
    }
}

static void
frameHandleDone(void* data, wl_callback* callback, u32 time)
{
    wl_callback_destroy(callback);
    drawFrame();
}

const wl_callback_listener frameListener = {
    .done = frameHandleDone,
};

static void
xdgSurfaceHandleConfigure(void* data, xdg_surface* xdg_surface, u32 serial)
{
    xdg_surface_ack_configure(xdg_surface, serial);
}

static const xdg_surface_listener xdg_surface_listener = {
    .configure = xdgSurfaceHandleConfigure,
};

static void
configureHandle(void* data, xdg_toplevel* xdgToplevel, s32 width, s32 height, wl_array* states)
{
    if (width != appState.windowWidth || height != appState.windowHeight)
    {
        wl_egl_window_resize(appState.eglWindow, width, height, 0, 0);
        appState.windowWidth = width;
        appState.windowHeight = height;

        D(glViewport(0, 0, width, height));
    }
}

static void
xdgToplevelHandleClose(void* data, xdg_toplevel* xdgToplevel)
{
    appState.programIsRunning = false;
}

static const xdg_toplevel_listener xdg_toplevel_listener = {
    .configure = configureHandle,
    .close = xdgToplevelHandleClose,
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
seatHandleCapabilities(void* data, wl_seat* seat, u32 capabilities)
{
    if (capabilities & WL_SEAT_CAPABILITY_POINTER)
    {
        appState.pointer = wl_seat_get_pointer(seat);
        wl_pointer_add_listener(appState.pointer, &pointerListener, seat);
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

static const wl_seat_listener seatListener = {
    .capabilities = seatHandleCapabilities,
};

static void
handleGlobal(void* data, wl_registry* registry, u32 name, const char* interface, u32 version)
{
    if (strcmp(interface, wl_seat_interface.name) == 0)
    {
        appState.seat = (wl_seat*)wl_registry_bind(registry, name, &wl_seat_interface, 1);
        wl_seat_add_listener(appState.seat, &seatListener, nullptr);
    }
    else if (strcmp(interface, wl_compositor_interface.name) == 0)
    {
        compositor = (wl_compositor*)wl_registry_bind(registry, name, &wl_compositor_interface, 1);
    }
    else if (strcmp(interface, xdg_wm_base_interface.name) == 0)
    {
        xdgWmBase = (xdg_wm_base*)wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
    }
    else if (strcmp(interface, zwp_pointer_constraints_v1_interface.name) == 0)
    {
        appState.pointerConstraints = (zwp_pointer_constraints_v1*)wl_registry_bind(registry, name, &zwp_pointer_constraints_v1_interface, version);
    }
    else if (strcmp(interface, zwp_relative_pointer_manager_v1_interface.name) == 0)
    {
        appState.relativePointerManager = (zwp_relative_pointer_manager_v1*)wl_registry_bind(registry, name, &zwp_relative_pointer_manager_v1_interface, version);
    }
    // else if (strcmp(interface, zwp_relative_pointer_v1_interface.name) == 0)
    // {
        // appState.relativePointer = (zwp_relative_pointer_v1*)wl_registry_bind(registry, name, &zwp_relative_pointer_v1_interface, version);
    // }
}

static void
handleGlobalRemove(void* data, wl_registry* registry, u32 name)
{
    // Who cares
}

static const wl_registry_listener registryListener = {
    .global = handleGlobal,
    .global_remove = handleGlobalRemove,
};

int
main(int argc, char* argv[])
{
    wl_display* display = wl_display_connect(nullptr);
    if (display == nullptr)
    {
        fprintf(stderr, "failed to create display\n");
        return EXIT_FAILURE;
    }

    wl_registry* registry = wl_display_get_registry(display);
    wl_registry_add_listener(registry, &registryListener, nullptr);
    wl_display_dispatch(display);
    wl_display_roundtrip(display);

    if (compositor == nullptr || xdgWmBase == nullptr)
    {
        fprintf(stderr, "no wl_shm, wl_compositor or xdg_wm_base support\n");
        return EXIT_FAILURE;
    }

    appState.eglDisplay = eglGetDisplay((EGLNativeDisplayType)display);
    EGLD();
    if (appState.eglDisplay == EGL_NO_DISPLAY)
    {
        fprintf(stderr, "failed to create EGL display\n");
        return EXIT_FAILURE;
    }

    EGLint major, minor;
    if (!eglInitialize(appState.eglDisplay, &major, &minor))
    {
        fprintf(stderr, "failed to initialize EGL\n");
        return EXIT_FAILURE;
    }
    EGLD();
    LOG(OK, "egl: major: {}, minor: {}\n", major, minor);

    EGLint count;
    EGLD(eglGetConfigs(appState.eglDisplay, nullptr, 0, &count));

    EGLint configAttribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
        // // EGL_MIN_SWAP_INTERVAL, 0,
        // EGL_MAX_SWAP_INTERVAL, 1,
        EGL_NONE,
    };
    EGLint n = 0;
    EGLConfig* configs = new EGLConfig[count] {};
    EGLD(eglChooseConfig(appState.eglDisplay, configAttribs, configs, count, &n));
    if (n == 0)
    {
        LOG(FATAL, "Failed to choose an EGL config\n");
    }
    EGLConfig eglConfig = configs[0];

    EGLint contextAttribs[] {
        EGL_CONTEXT_CLIENT_VERSION,
        3,
        EGL_NONE,
    };
    appState.eglContext = eglCreateContext(appState.eglDisplay, eglConfig, EGL_NO_CONTEXT, contextAttribs);
    EGLD();

    appState.surface = wl_compositor_create_surface(compositor);
    appState.xdgSurface = xdg_wm_base_get_xdg_surface(xdgWmBase, appState.surface);
    appState.xdgToplevel = xdg_surface_get_toplevel(appState.xdgSurface);

    std::vector<char> nameStr = loadFileToStr("name", 1);
    nameStr[nameStr.size() - 2] = '\0'; /* remove '\n' */
    appState.nameStr = nameStr.data();

    xdg_toplevel_set_title(appState.xdgToplevel, appState.nameStr.data());
    xdg_toplevel_set_app_id(appState.xdgToplevel, appState.nameStr.data());

    xdg_surface_add_listener(appState.xdgSurface, &xdg_surface_listener, nullptr);
    xdg_toplevel_add_listener(appState.xdgToplevel, &xdg_toplevel_listener, nullptr);

    appState.eglWindow = wl_egl_window_create(appState.surface, appState.windowWidth, appState.windowHeight);
    EGLD(appState.eglSurface = eglCreateWindowSurface(appState.eglDisplay, eglConfig, (EGLNativeWindowType)appState.eglWindow, nullptr));

    wl_surface_commit(appState.surface);
    wl_display_roundtrip(display);

    // wl_event_queue* q = wl_display_create_queue(display);

    // Draw the first frame
    setupDraw();
    drawFrame();

    while (wl_display_dispatch(display) != -1 && appState.programIsRunning)
    {
        // This space intentionally left blank
    }

    fprintf(stderr, "cleanup ...\n");
    xdg_toplevel_destroy(appState.xdgToplevel);
    xdg_surface_destroy(appState.xdgSurface);
    wl_surface_destroy(appState.surface);

    wl_egl_window_destroy(appState.eglWindow);
    EGLD(eglDestroySurface(appState.eglDisplay, appState.eglSurface));
    EGLD(eglDestroyContext(appState.eglDisplay, appState.eglContext));

    delete[] configs;

    return EXIT_SUCCESS;
}
