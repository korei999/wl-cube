#include "headers/main.hh"
#include "headers/math.hh"
#include "headers/utils.hh"
#include "headers/shader.hh"

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <cstdio>
#include <ctime>
#include <wayland-client-protocol.h>
#include <print>

static void swapFrames();

Shader simpleShader;

GLfloat vVertices[] = {
     0.0f,  0.5f, 0.0f,
    -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f
};

static void
setupShaders()
{
    simpleShader = {"shaders/ht.vert", "shaders/ht.frag"};
}

void
loadVertices()
{
    D( glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vVertices) );
    D( glEnableVertexAttribArray(0) );
}

void
setupDraw()
{
    if (!eglMakeCurrent(appState.eglDisplay, appState.eglSurface, appState.eglSurface, appState.eglContext))
        LOG(FATAL, "eglMakeCurrent failed\n");

    // By default, eglSwapBuffers blocks until we receive the next frame event.
    // This is undesirable since it makes it impossible to process other events
    // (such as input events) while waiting for the next frame event. Setting
    // the swap interval to zero and managing frame events manually prevents
    // this behavior.
    EGLD( eglSwapInterval(appState.eglDisplay, 0) );

    setupShaders();

    // D( glEnable(GL_DEPTH_TEST) );

    v4 gray = COLOR(0x121212ff);
    D( glClearColor(gray.r, gray.g, gray.b, gray.a) );

    loadVertices();
}

void
drawFrame(void)
{
    D( glClear(GL_COLOR_BUFFER_BIT) );

    static f32 inc = 0;

    m4 tm = m4Identity();
    tm = m4RotX(tm, TO_RAD(inc));
    tm = m4Trans(tm, {sin(inc/10)/2, 0, 0});

    simpleShader.use();
    simpleShader.setMat4("transform", tm);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    inc += 0.3;
    swapFrames();
}

static void
swapFrames()
{
    // Register a frame callback to know when we need to draw the next frame
    wl_callback* callback = wl_surface_frame(appState.surface);
    wl_callback_add_listener(callback, &frameListener, NULL);

    // This will attach a new buffer and commit the surface
    if (!eglSwapBuffers(appState.eglDisplay, appState.eglSurface))
        LOG(FATAL, "eglSwapBuffers failed\n");
}
