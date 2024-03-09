#include "headers/main.hh"
#include "headers/mmath.hh"
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
    if (!eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context))
        LOG(FATAL, "eglMakeCurrent failed\n");

    // By default, eglSwapBuffers blocks until we receive the next frame event.
    // This is undesirable since it makes it impossible to process other events
    // (such as input events) while waiting for the next frame event. Setting
    // the swap interval to zero and managing frame events manually prevents
    // this behavior.
    EGLD( eglSwapInterval(egl_display, 0) );

    setupShaders();

    // D( glEnable(GL_DEPTH_TEST) );

    GLfloat gray[] = COLOR(0x121212ff);
    D( glClearColor(gray[0], gray[1], gray[2], gray[3]) );

    loadVertices();
}

void
drawFrame(void)
{
    D( glClear(GL_COLOR_BUFFER_BIT) );

    simpleShader.use();

    m4 tm = IDENT;

    static f32 inc = 0;

    v3 rot {0.5f, 0.5f, 0.5f};
    vec3Norm(rot);
    mat4Rot(tm, TO_RAD(inc), rot);

    inc += 0.5;
    simpleShader.setMat4("transform", tm);

    D( glDrawArrays(GL_TRIANGLES, 0, 3) );

    swapFrames();
}

static void
swapFrames()
{
    // Register a frame callback to know when we need to draw the next frame
    struct wl_callback* callback = wl_surface_frame(surface);
    wl_callback_add_listener(callback, &frame_listener, NULL);

    // This will attach a new buffer and commit the surface
    if (!eglSwapBuffers(egl_display, egl_surface))
        LOG(FATAL, "eglSwapBuffers failed\n");
}
