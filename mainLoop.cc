#include "headers/controls.hh"
#include "headers/main.hh"
#include "headers/math.hh"
#include "headers/shader.hh"
#include "headers/utils.hh"

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <cstdio>
#include <ctime>
#include <wayland-client-protocol.h>
#include <print>
#include <unistd.h>

static void swapFrames();

Shader simpleShader;

PlayerControls player {
    .mouse.sens = 0.07,
    .pos {0.0, 0.0, 0.1},
    .moveSpeed = 0.2,
};

GLfloat vCube[][4] {
    /* front */
    -0.5f,  0.5f, -0.5f, 1.0f,
    -0.5f, -0.5f, -0.5f, 1.0f,
    0.5f, -0.5f,  -0.5f, 1.0f,

    0.5f,  0.5f,  -0.5f, 1.0f,
    -0.5f,  0.5f, -0.5f, 1.0f,
    0.5f, -0.5f,  -0.5f, 1.0f,
    /* left */
    -0.5f,  0.5f, -0.5f, 1.0f,
    -0.5f,  0.5f, 0.5f, 1.0f,
    -0.5f, -0.5f, -0.5f, 1.0f,

    -0.5f,  -0.5f, 0.5f, 1.0f,
    -0.5f,  0.5f,  0.5f, 1.0f,
    -0.5f, -0.5f,  -0.5f, 1.0f,
    /* right */
    0.5f, -0.5f, -0.5f, 1.0f,
    0.5f, 0.5f,  -0.5f, 1.0f,
    0.5f, 0.5f,  0.5f, 1.0f,

    0.5f, -0.5f, -0.5f, 1.0f,
    0.5f, -0.5f, 0.5f, 1.0f,
    0.5f, 0.5f,  0.5f, 1.0f,
    /* back */
    -0.5f,  0.5f, 0.5f, 1.0f,
    -0.5f, -0.5f, 0.5f, 1.0f,
    0.5f, -0.5f,  0.5f, 1.0f,

    0.5f,  0.5f,  0.5f, 1.0f,
    -0.5f,  0.5f, 0.5f, 1.0f,
    0.5f, -0.5f,  0.5f, 1.0f,
    /* top */
    -0.5f, 0.5f, -0.5f, 1.0f,
    -0.5f, 0.5f, 0.5f, 1.0f,
    0.5f, 0.5f,  -0.5f, 1.0f,

    0.5f, 0.5f,  -0.5f, 1.0f,
    0.5f, 0.5f,  0.5f, 1.0f,
    -0.5f, 0.5f, 0.5f, 1.0f,
    /* bottom */
    -0.5f, -0.5f, -0.5f, 1.0f,
    -0.5f, -0.5f, 0.5f, 1.0f,
    0.5f,  -0.5f, -0.5f, 1.0f,

    0.5f,  -0.5f, -0.5f, 1.0f,
    0.5f,  -0.5f, 0.5f, 1.0f,
    -0.5f, -0.5f, 0.5f, 1.0f,
};

static void
setupShaders()
{
    simpleShader = {"shaders/ht.vert", "shaders/ht.frag"};
    simpleShader.queryActiveUniforms();
}

GLuint posBufferObj;

void
loadVertices()
{
    // bindless option
    // D( glEnableVertexAttribArray(0) );
    // D( glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, vCube) );

    D( glGenBuffers(1, &posBufferObj) );
    D( glBindBuffer(GL_ARRAY_BUFFER, posBufferObj) );
    D( glBufferData(GL_ARRAY_BUFFER, sizeof(vCube), vCube, GL_STATIC_DRAW) );
    D( glEnableVertexAttribArray(0) );
    D( glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0) );
}

void
setupDraw()
{
    if (!eglMakeCurrent(appState.eglDisplay, appState.eglSurface, appState.eglSurface, appState.eglContext))
        LOG(FATAL, "eglMakeCurrent failed\n");

    EGLD( eglSwapInterval(appState.eglDisplay, 0) );

    setupShaders();
    appState.togglePointerRelativeMode();

    D( glEnable(GL_DEPTH_TEST) );
    D( glDepthFunc(GL_LESS) );

    v4 gray = COLOR(0x121212ff);
    D( glClearColor(gray.r, gray.g, gray.b, gray.a) );

    loadVertices();
}

void
drawFrame(void)
{
    player.updateDeltaTime();
    player.procMouse();
    player.procKeys();

    if (!appState.paused)
    {
        D( glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) );

        m4 proj = m4Pers(90.0f, (f32)appState.windowWidth / (f32)appState.windowHeight, 0.01f, 100.0f);
        m4 view = m4LookAt(player.pos, player.pos + player.front, player.up);
        m4 tm = m4Iden();

        static f64 inc = 0;
        tm = m4Scale(tm, 0.05f);

        simpleShader.use();
        simpleShader.setM4("proj", proj);
        simpleShader.setM4("view", view);

        for (int i = 0; i < 20; i++)
        {
            tm = m4Trans(tm, {0, (f32)i, 0});
            tm = m4Rot(tm, TO_RAD(inc), v3Norm({0.25f, 0.25f, 0.75f}));

            simpleShader.setM4("model", tm);
            D( glDrawArrays(GL_TRIANGLES, 0, LEN(vCube)) );
        }

        inc += 0.01;
    }
    swapFrames();
}

static void
swapFrames()
{
    /* Register a frame callback to know when we need to draw the next frame */
    wl_callback* callback = wl_surface_frame(appState.surface);
    wl_callback_add_listener(callback, &frameListener, NULL);

    /* This will attach a new buffer and commit the surface */
    if (!eglSwapBuffers(appState.eglDisplay, appState.eglSurface))
        LOG(FATAL, "eglSwapBuffers failed\n");
}
