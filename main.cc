#define _POSIX_C_SOURCE 199309L
#include "headers/main.hh"
#include "headers/utils.hh"
#include "headers/input.hh"

#include <cstring>

AppState appState {
    .wWidth = 1280,
    .wHeight = 720,
};

[[maybe_unused]] enum
{
	REGION_TYPE_NONE,
	REGION_TYPE_DISJOINT,
	REGION_TYPE_JOINT,
	REGION_TYPE_MAX
} region_type = REGION_TYPE_NONE;

static const zwp_relative_pointer_v1_listener relativePointerListener {
	.relative_motion = relativePointerMotionHandler
};

static void
frameDoneHandler([[maybe_unused]] void* data,
                 [[maybe_unused]] wl_callback* callback,
                 [[maybe_unused]] u32 time)
{
    wl_callback_destroy(callback);
    drawFrame();
}

const wl_callback_listener frameListener {
    .done = frameDoneHandler,
};

static void
xdgSurfaceConfigureHandler([[maybe_unused]] void* data,
                           [[maybe_unused]] xdg_surface* xdg_surface,
                           [[maybe_unused]] u32 serial)
{ 
    xdg_surface_ack_configure(xdg_surface, serial);
}

static const xdg_surface_listener xdgSurfaceListener {
    .configure = xdgSurfaceConfigureHandler,
};

static void
xdgToplevelConfigureHandler([[maybe_unused]] void* data,
                            [[maybe_unused]] xdg_toplevel* xdgToplevel,
                            [[maybe_unused]] s32 width,
                            [[maybe_unused]] s32 height,
                            [[maybe_unused]] wl_array* states)
{
    if (width > 0 && height > 0)
    {
        if (width != appState.wWidth || height != appState.wHeight)
        {
            wl_egl_window_resize(appState.eglWindow, width, height, 0, 0);
            appState.wWidth = width;
            appState.wHeight = height;

            D( glViewport(0, 0, width, height) );
        }
    }
}

static void
xdgToplevelCloseHandler([[maybe_unused]] void* data,
                        [[maybe_unused]] xdg_toplevel* xdgToplevel)
{
    appState.programIsRunning = false;
}

static void
xdgToplevelConfigureBounds([[maybe_unused]] void* data,
				           [[maybe_unused]] xdg_toplevel* toplevel,
				           [[maybe_unused]] s32 width,
                           [[maybe_unused]] s32 height)
{
    LOG(OK, "configure_bounds: width: {}, height: {}\n", width, height);
}

static const xdg_toplevel_listener xdgToplevelListener = {
    .configure = xdgToplevelConfigureHandler,
    .close = xdgToplevelCloseHandler,
    .configure_bounds = xdgToplevelConfigureBounds
};

static const wl_pointer_listener pointerListener {
    .enter = pointerEnterHandler,
    .leave = pointerLeaveHandler,
    .motion = pointerMotionHandler,
    .button = pointerButtonHandler,
    .axis = pointerAxisHandler,
};

static const wl_keyboard_listener keyboardListener {
    .keymap = keyboardKeymapHandler,
    .enter = keyboardEnterHandler,
    .leave = keyboardLeaveHandler,
    .key = keyboardKeyHandler,
    .modifiers = keyboardModifiersHandler,
    .repeat_info = keyboardRepeatInfoHandler
};

void
AppState::setCursor(std::string_view cursorType)
{
    wl_cursor* cursor = wl_cursor_theme_get_cursor(cursorTheme, cursorType.data());
    cursorImage = cursor->images[0];
    wl_buffer* cursorBuffer = wl_cursor_image_get_buffer(cursorImage);

    cursorSurface = wl_compositor_create_surface(compositor);
    wl_pointer_set_cursor(pointer, pointerSerial, cursorSurface, 0, 0);
    wl_surface_attach(cursorSurface, cursorBuffer, 0, 0);
    wl_surface_commit(cursorSurface);

    wl_pointer_set_cursor(pointer, pointerSerial, cursorSurface, cursorImage->hotspot_x, cursorImage->hotspot_y);
}

void
AppState::enableRelativeMode()
{
    wl_pointer_set_cursor(pointer, pointerSerial, nullptr, 0, 0);
    lockedPointer = zwp_pointer_constraints_v1_lock_pointer(pointerConstraints,
                                                            surface,
                                                            pointer,
                                                            nullptr,
                                                            ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_ONESHOT);
    relativePointer = zwp_relative_pointer_manager_v1_get_relative_pointer(relativePointerManager, pointer);
    zwp_relative_pointer_v1_add_listener(relativePointer, &relativePointerListener, nullptr);
}

void
AppState::disableRelativeMode()
{
    zwp_locked_pointer_v1_destroy(lockedPointer);
    zwp_relative_pointer_v1_destroy(relativePointer);

    setCursor();
}

void 
AppState::setFullscreen()
{
    xdg_toplevel_set_fullscreen(xdgToplevel, output);
}

void
AppState::unsetFullscreen()
{
    xdg_toplevel_unset_fullscreen(xdgToplevel);
}

static void
seatCapabilitiesHandler([[maybe_unused]] void* data,
                        [[maybe_unused]] wl_seat* seat,
                        [[maybe_unused]] u32 capabilities)
{
    if (capabilities & WL_SEAT_CAPABILITY_POINTER)
    {
        appState.pointer = wl_seat_get_pointer(seat);
        appState.cursorTheme = wl_cursor_theme_load(nullptr, 24, appState.shm);
        wl_pointer_add_listener(appState.pointer, &pointerListener, seat);
        LOG(GOOD, "pointer works.\n");
    }
    if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD)
    {
        appState.keyboard = wl_seat_get_keyboard(seat);
        wl_keyboard_add_listener(appState.keyboard, &keyboardListener, seat);
        LOG(GOOD, "keyboard works.\n");
    }
    if (capabilities & WL_SEAT_CAPABILITY_TOUCH)
    {
        //
        LOG(GOOD, "touch works.\n");
    }
}

static const wl_seat_listener seatListener {
    .capabilities = seatCapabilitiesHandler,
};

static void
xdgWmBasePingHandler(void *data,
		     struct xdg_wm_base *xdg_wm_base,
		     uint32_t serial)
{
    xdg_wm_base_pong(appState.xdgWmBase, serial);
}


static xdg_wm_base_listener xdgWmBaseListener {
    .ping = xdgWmBasePingHandler
};

static void
handleGlobal([[maybe_unused]] void* data,
             [[maybe_unused]] wl_registry* registry,
             [[maybe_unused]] u32 name,
             [[maybe_unused]] const char* interface,
             [[maybe_unused]] u32 version)
{
    LOG(GOOD, "interface: '{}', version: {}, name: {}\n", interface, version, name);

    if (strcmp(interface, wl_seat_interface.name) == 0)
    {
        appState.seat = (wl_seat*)wl_registry_bind(registry, name, &wl_seat_interface, 1);
        wl_seat_add_listener(appState.seat, &seatListener, nullptr);
    }
    else if (strcmp(interface, wl_compositor_interface.name) == 0)
    {
        appState.compositor = (wl_compositor*)wl_registry_bind(registry, name, &wl_compositor_interface, 1);
    }
    else if (strcmp(interface, xdg_wm_base_interface.name) == 0)
    {
        appState.xdgWmBase = (xdg_wm_base*)wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
        xdg_wm_base_add_listener(appState.xdgWmBase, &xdgWmBaseListener, nullptr);
    }
    else if (strcmp(interface, zwp_pointer_constraints_v1_interface.name) == 0)
    {
        appState.pointerConstraints = (zwp_pointer_constraints_v1*)wl_registry_bind(registry, name, &zwp_pointer_constraints_v1_interface, version);
    }
    else if (strcmp(interface, zwp_relative_pointer_manager_v1_interface.name) == 0)
    {
        appState.relativePointerManager = (zwp_relative_pointer_manager_v1*)wl_registry_bind(registry, name, &zwp_relative_pointer_manager_v1_interface, version);
    }
    else if (strcmp(interface, wl_shm_interface.name) == 0)
    {
        appState.shm = (wl_shm*)wl_registry_bind(registry, name, &wl_shm_interface, 1);
    }
    else if (strcmp(interface, wl_output_interface.name) == 0)
    {
        appState.output = (wl_output*)wl_registry_bind(registry, name, &wl_output_interface, version);
    }
}

static void
handleGlobalRemove([[maybe_unused]] void* data,
                   [[maybe_unused]] wl_registry* registry,
                   [[maybe_unused]] u32 name)
{
    // Who cares
}

static const wl_registry_listener registryListener {
    .global = handleGlobal,
    .global_remove = handleGlobalRemove,
};


void
swapFrames()
{
    /* Register a frame callback to know when we need to draw the next frame */
    wl_callback* callback = wl_surface_frame(appState.surface);
    wl_callback_add_listener(callback, &frameListener, nullptr);
    // wl_surface_damage(appState.surface, appState.wWidth, appState.wHeight, 0, 0);

    /* This will attach a new buffer and commit the surface */
    if (!eglSwapBuffers(appState.eglDisplay, appState.eglSurface))
        LOG(FATAL, "eglSwapBuffers failed\n");
}

int
main()
{
    appState.display = wl_display_connect(nullptr);
    if (appState.display == nullptr)
    {
        fprintf(stderr, "failed to create display\n");
        return EXIT_FAILURE;
    }

    wl_registry* registry = wl_display_get_registry(appState.display);
    wl_registry_add_listener(registry, &registryListener, nullptr);
    wl_display_dispatch(appState.display);
    wl_display_roundtrip(appState.display);

    if (appState.compositor == nullptr || appState.xdgWmBase == nullptr)
    {
        fprintf(stderr, "no wl_shm, wl_compositor or xdg_wm_base support\n");
        return EXIT_FAILURE;
    }

    appState.eglDisplay = eglGetDisplay((EGLNativeDisplayType)appState.display);
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
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
        // // EGL_MIN_SWAP_INTERVAL, 0,
        // EGL_MAX_SWAP_INTERVAL, 1,
        EGL_NONE,
    };
    EGLint n = 0;
    // EGLConfig* configs = new EGLConfig[count] {};
    std::vector<EGLConfig> configs;// = new EGLConfig[count] {};
    configs.resize(count);

    EGLD(eglChooseConfig(appState.eglDisplay, configAttribs, configs.data(), count, &n));
    if (n == 0)
    {
        LOG(FATAL, "Failed to choose an EGL config\n");
    }
    EGLConfig eglConfig = configs[0];

    EGLint contextAttribs[] {
        EGL_CONTEXT_CLIENT_VERSION, 3,
        EGL_NONE,
    };
    appState.eglContext = eglCreateContext(appState.eglDisplay, eglConfig, EGL_NO_CONTEXT, contextAttribs);
    EGLD();

    appState.surface = wl_compositor_create_surface(appState.compositor);
    appState.xdgSurface = xdg_wm_base_get_xdg_surface(appState.xdgWmBase, appState.surface);
    appState.xdgToplevel = xdg_surface_get_toplevel(appState.xdgSurface);

    std::vector<char> nameStr = fileLoad("name", 1);
    nameStr[nameStr.size() - 2] = '\0'; /* remove '\n' */
    appState.nameStr = nameStr.data();

    xdg_toplevel_set_title(appState.xdgToplevel, appState.nameStr.data());
    xdg_toplevel_set_app_id(appState.xdgToplevel, appState.nameStr.data());

    xdg_surface_add_listener(appState.xdgSurface, &xdgSurfaceListener, nullptr);
    xdg_toplevel_add_listener(appState.xdgToplevel, &xdgToplevelListener, nullptr);

    appState.eglWindow = wl_egl_window_create(appState.surface, appState.wWidth, appState.wHeight);
    EGLD(appState.eglSurface = eglCreateWindowSurface(appState.eglDisplay, eglConfig, (EGLNativeWindowType)appState.eglWindow, nullptr));

    appState.isRelativeMode = true;

    wl_surface_commit(appState.surface);
    wl_display_roundtrip(appState.display);

    appState.programIsRunning = true;
    appState.isRelativeMode = true;

    // Draw the first frame
    setupDraw();
    drawFrame();

    while (wl_display_dispatch(appState.display) != -1 && appState.programIsRunning)
    {
        // This space intentionally left blank
    }

    return EXIT_SUCCESS;
}
