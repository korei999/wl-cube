#define _POSIX_C_SOURCE 199309L

#include "input.hh"
#include "../../utils.hh"
#include "wayland.hh"

#include <cstring>

EGLint eglLastErrorCode = EGL_SUCCESS;

#ifdef DEBUG
#    define EGLD(C)                                                                                                    \
        {                                                                                                              \
            C;                                                                                                         \
            if ((eglLastErrorCode = eglGetError()) != EGL_SUCCESS)                                                     \
                LOG(FATAL, "eglLastErrorCode: {:#x}\n", eglLastErrorCode);                                             \
        }
#else
#    define EGLD(C) C
#endif

static const zwp_relative_pointer_v1_listener relativePointerListener {
	.relative_motion = relativePointerMotionHandler
};

static const wl_pointer_listener pointerListener {
    .enter = pointerEnterHandler,
    .leave = pointerLeaveHandler,
    .motion = pointerMotionHandler,
    .button = pointerButtonHandler,
    .axis = pointerAxisHandler,
    .frame {},
    .axis_source {},
    .axis_stop {},
    .axis_discrete {},
    .axis_value120 {},
    .axis_relative_direction {}
};

static const wl_keyboard_listener keyboardListener {
    .keymap = keyboardKeymapHandler,
    .enter = keyboardEnterHandler,
    .leave = keyboardLeaveHandler,
    .key = keyboardKeyHandler,
    .modifiers = keyboardModifiersHandler,
    .repeat_info = keyboardRepeatInfoHandler
};

/* mutter compositor will complain if we do not pong */
static void
xdgWmBasePing([[maybe_unused]] void* data,
              [[maybe_unused]] xdg_wm_base* xdgWmBase,
              [[maybe_unused]] u32 serial)
{
    auto app = (WlClient*)data;
    xdg_wm_base_pong(app->xdgWmBase, serial);
}

static void
seatCapabilitiesHandler([[maybe_unused]] void* data,
                        [[maybe_unused]] wl_seat* seat,
                        [[maybe_unused]] u32 capabilities)
{
    auto app = (WlClient*)data;

    if (capabilities & WL_SEAT_CAPABILITY_POINTER)
    {
        app->pointer = wl_seat_get_pointer(app->seat);
        app->cursorTheme = wl_cursor_theme_load(nullptr, 24, app->shm);
        wl_pointer_add_listener(app->pointer, &pointerListener, app);
        LOG(GOOD, "pointer works.\n");
    }
    if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD)
    {
        app->keyboard = wl_seat_get_keyboard(seat);
        wl_keyboard_add_listener(app->keyboard, &keyboardListener, app);
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
	.name = {}
};

static void
xdgSurfaceConfigureHandler([[maybe_unused]] void* data,
                           [[maybe_unused]] xdg_surface* xdgSurface,
                           [[maybe_unused]] u32 serial)
{
    auto app = (WlClient*)data;
    xdg_surface_ack_configure(xdgSurface, serial);
    app->bConfigured = true;
}

static void
xdgToplevelConfigureHandler([[maybe_unused]] void* data,
                            [[maybe_unused]] xdg_toplevel* xdgToplevel,
                            [[maybe_unused]] s32 width,
                            [[maybe_unused]] s32 height,
                            [[maybe_unused]] wl_array* states)
{
    auto app = (WlClient*)data;

    if (width > 0 && height > 0)
    {
        if (width != app->wWidth || height != app->wHeight)
        {
            wl_egl_window_resize(app->eglWindow, width, height, 0, 0);
            app->wWidth = width;
            app->wHeight = height;
        }
    }
}

static void
xdgToplevelCloseHandler([[maybe_unused]] void* data,
		                [[maybe_unused]] xdg_toplevel* xdgToplevel)
{
    auto app = (WlClient*)data;
    app->bRunning = false;
    LOG(OK, "closing...\n");
}

static void
xdgToplevelConfigureBounds([[maybe_unused]] void* data,
                           [[maybe_unused]] xdg_toplevel* xdgToplevel,
                           [[maybe_unused]] s32 width,
                           [[maybe_unused]] s32 height)
{
    //
}

static const xdg_surface_listener xdgSurfaceListener {
    .configure = xdgSurfaceConfigureHandler
};

static const xdg_wm_base_listener xdgWmBaseListener {
    .ping = xdgWmBasePing
};


static void
xdgToplevelWmCapabilities([[maybe_unused]] void* data,
                          [[maybe_unused]] xdg_toplevel* xdgToplevel,
                          [[maybe_unused]] wl_array* capabilities)
{
    //
}

static const xdg_toplevel_listener xdgToplevelListener {
    .configure = xdgToplevelConfigureHandler,
    .close = xdgToplevelCloseHandler,
    .configure_bounds = xdgToplevelConfigureBounds,
    .wm_capabilities = xdgToplevelWmCapabilities
};

static void
registryGlobalHandler([[maybe_unused]] void* data,
                      [[maybe_unused]] wl_registry* registry,
                      [[maybe_unused]] u32 name,
                      [[maybe_unused]] const char* interface,
                      [[maybe_unused]] u32 version)
{
    LOG(OK, "interface: '{}', version: {}, name: {}\n", interface, version, name);
    auto app = (WlClient*)data;

    if (strcmp(interface, wl_seat_interface.name) == 0)
    {
        app->seat = (wl_seat*)wl_registry_bind(registry, name, &wl_seat_interface, 1);
        wl_seat_add_listener(app->seat, &seatListener, app);
    }
    else if (strcmp(interface, wl_compositor_interface.name) == 0)
    {
        app->compositor = (wl_compositor*)wl_registry_bind(registry, name, &wl_compositor_interface, version);
    }
    else if (strcmp(interface, xdg_wm_base_interface.name) == 0)
    {
        app->xdgWmBase = (xdg_wm_base*)wl_registry_bind(registry, name, &xdg_wm_base_interface, version);
        xdg_wm_base_add_listener(app->xdgWmBase, &xdgWmBaseListener, app);
    }
    else if (strcmp(interface, zwp_pointer_constraints_v1_interface.name) == 0)
    {
        app->pointerConstraints = (zwp_pointer_constraints_v1*)wl_registry_bind(registry, name, &zwp_pointer_constraints_v1_interface, version);
    }
    else if (strcmp(interface, zwp_relative_pointer_manager_v1_interface.name) == 0)
    {
        app->relativePointerManager = (zwp_relative_pointer_manager_v1*)wl_registry_bind(registry, name, &zwp_relative_pointer_manager_v1_interface, version);
    }
    else if (strcmp(interface, wl_shm_interface.name) == 0)
    {
        app->shm = (wl_shm*)wl_registry_bind(registry, name, &wl_shm_interface, version);
    }
    else if (strcmp(interface, wl_output_interface.name) == 0)
    {
        app->output = (wl_output*)wl_registry_bind(registry, name, &wl_output_interface, version);
    }
}

static void
registryGlobalRemoveHandler([[maybe_unused]] void* data,
                            [[maybe_unused]] wl_registry* wlRegistry,
                            [[maybe_unused]] u32 name)
{
    /* can be empty */
}

static const wl_registry_listener registryListener {
    .global = registryGlobalHandler,
    .global_remove = registryGlobalRemoveHandler
};

WlClient::WlClient(std::string_view name)
{
    this->name = name;
    init();
}

WlClient::~WlClient()
{
    LOG(OK, "cleanup ...\n");
    if (bRelativeMode)
        disableRelativeMode();
    if (pointer)
        wl_pointer_destroy(pointer);
    if (cursorTheme)
        wl_cursor_theme_destroy(cursorTheme);
    if (keyboard)
        wl_keyboard_destroy(keyboard);
    if (xdgSurface)
        xdg_surface_destroy(xdgSurface);
    if (xdgToplevel)
        xdg_toplevel_destroy(xdgToplevel);
    if (surface)
        wl_surface_destroy(surface);
    if (shm)
        wl_shm_destroy(shm);
    if (xdgWmBase)
        xdg_wm_base_destroy(xdgWmBase);
    if (pointerConstraints)
        zwp_pointer_constraints_v1_destroy(pointerConstraints);
    if (relativePointerManager)
        zwp_relative_pointer_manager_v1_destroy(relativePointerManager);
    if (cursorSurface)
        wl_surface_destroy(cursorSurface);
    if (seat)
        wl_seat_destroy(seat);
    if (output)
        wl_output_destroy(output);
    if (compositor)
        wl_compositor_destroy(compositor);
    if (registry)
        wl_registry_destroy(registry);
}

void
WlClient::init()
{
    if ((display = wl_display_connect(nullptr)))
        LOG(GOOD, "wayland display connected\n");
    else
        LOG(FATAL, "error connecting wayland display\n");

    registry = wl_display_get_registry(display);
    wl_registry_add_listener(registry, &registryListener, this);
    wl_display_dispatch(display);
    wl_display_roundtrip(display);

    if (compositor == nullptr || xdgWmBase == nullptr)
        LOG(FATAL, "no wl_shm, wl_compositor or xdg_wm_base support\n");

    EGLD( eglDisplay = eglGetDisplay((EGLNativeDisplayType)display) );
    if (eglDisplay == EGL_NO_DISPLAY)
        LOG(FATAL, "failed to create EGL display\n");

    EGLint major, minor;
    if (!eglInitialize(eglDisplay, &major, &minor))
        LOG(FATAL, "failed to initialize EGL\n");
    EGLD();

    LOG(OK, "egl: major: {}, minor: {}\n", major, minor);

    EGLint count;
    EGLD(eglGetConfigs(eglDisplay, nullptr, 0, &count));

    EGLint configAttribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        // EGL_ALPHA_SIZE, 8, /* KDE makes window transparent even in fullscreen */
        EGL_DEPTH_SIZE, 24,
        EGL_STENCIL_SIZE, 8,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
        EGL_SAMPLE_BUFFERS, 1,
        EGL_SAMPLES, 4,
        EGL_NONE
    };

    EGLint n = 0;
    std::vector<EGLConfig> configs(count);
    EGLD( eglChooseConfig(eglDisplay, configAttribs, configs.data(), count, &n) );
    if (n == 0)
        LOG(FATAL, "Failed to choose an EGL config\n");

    EGLConfig eglConfig = configs[0];

    EGLint contextAttribs[] {
        EGL_CONTEXT_CLIENT_VERSION, 3,
#ifdef DEBUG
        EGL_CONTEXT_OPENGL_DEBUG, EGL_TRUE,
#endif
        EGL_NONE,
    };

    EGLD( eglContext = eglCreateContext(eglDisplay, eglConfig, EGL_NO_CONTEXT, contextAttribs) );

    surface = wl_compositor_create_surface(compositor);
    xdgSurface = xdg_wm_base_get_xdg_surface(xdgWmBase, surface);
    xdgToplevel = xdg_surface_get_toplevel(xdgSurface);

    xdg_toplevel_set_title(xdgToplevel, name.data());
    xdg_toplevel_set_app_id(xdgToplevel, name.data());

    xdg_surface_add_listener(xdgSurface, &xdgSurfaceListener, this);
    xdg_toplevel_add_listener(xdgToplevel, &xdgToplevelListener, this);

    eglWindow = wl_egl_window_create(surface, wWidth, wHeight);
    EGLD( eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, (EGLNativeWindowType)eglWindow, nullptr) );

    wl_surface_commit(surface);
    wl_display_roundtrip(display);
}

void
WlClient::enableRelativeMode()
{
    wl_pointer_set_cursor(pointer, pointerSerial, nullptr, 0, 0);
    cursorSurface ? wl_surface_destroy(cursorSurface) : (void)0;
    lockedPointer = zwp_pointer_constraints_v1_lock_pointer(pointerConstraints,
                                                            surface,
                                                            pointer,
                                                            nullptr,
                                                            ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_ONESHOT);
    relativePointer = zwp_relative_pointer_manager_v1_get_relative_pointer(relativePointerManager, pointer);
    zwp_relative_pointer_v1_add_listener(relativePointer, &relativePointerListener, this);
}

void
WlClient::disableRelativeMode()
{
    zwp_locked_pointer_v1_destroy(lockedPointer);
    zwp_relative_pointer_v1_destroy(relativePointer);

    setCursorImage("default");
}

void
WlClient::setCursorImage(std::string_view cursorType)
{
    wl_cursor* cursor = wl_cursor_theme_get_cursor(cursorTheme, cursorType.data());
    if (!cursor)
    {
        LOG(WARNING, "failed to set cursor to '{}', falling back to 'default'\n", cursorType);
        cursor = wl_cursor_theme_get_cursor(cursorTheme, "default");
    }

    if (cursor)
    {
        cursorImage = cursor->images[0];
        wl_buffer* cursorBuffer = wl_cursor_image_get_buffer(cursorImage);

        cursorSurface = wl_compositor_create_surface(compositor);
        wl_pointer_set_cursor(pointer, pointerSerial, cursorSurface, 0, 0);
        wl_surface_attach(cursorSurface, cursorBuffer, 0, 0);
        wl_surface_commit(cursorSurface);

        wl_pointer_set_cursor(pointer, pointerSerial, cursorSurface, cursorImage->hotspot_x, cursorImage->hotspot_y);
    }
}

void 
WlClient::setFullscreen()
{
    xdg_toplevel_set_fullscreen(xdgToplevel, output);
}

void
WlClient::unsetFullscreen()
{
    xdg_toplevel_unset_fullscreen(xdgToplevel);
}

void
WlClient::togglePointerRelativeMode()
{
    bRelativeMode = !bRelativeMode;
    bRelativeMode ? enableRelativeMode() : disableRelativeMode();
    LOG(OK, "relative mode: {}\n", bRelativeMode);
}

void
WlClient::toggleFullscreen()
{
    bFullscreen = !bFullscreen;
    bFullscreen ? setFullscreen() : unsetFullscreen();
}

void 
WlClient::bindGlContext()
{
    EGLD ( eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext) );
}

void 
WlClient::unbindGlContext()
{
    EGLD( eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT) );
}

void
WlClient::setSwapInterval(int interval)
{
    swapInterval = interval;
    EGLD( eglSwapInterval(eglDisplay, interval) );
}

void
WlClient::toggleVSync()
{
    swapInterval = !swapInterval;
    EGLD( eglSwapInterval(eglDisplay, swapInterval) );
    LOG(OK, "swapInterval: {}\n", swapInterval);
}

void
WlClient::swapBuffers()
{
    EGLD( eglSwapBuffers(eglDisplay, eglSurface) );
    if (wl_display_dispatch(display) == -1)
        LOG(FATAL, "wl_display_dispatch error\n");
}

void 
WlClient::procEvents()
{
    //
}

void
WlClient::showWindow()
{
    //
}
