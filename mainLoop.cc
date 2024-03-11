#include "headers/main.hh"
#include "headers/math.hh"
#include "headers/utils.hh"
#include "headers/shader.hh"
#include "headers/input.hh"

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <cstdio>
#include <ctime>
#include <wayland-client-protocol.h>
#include <print>
#include <unistd.h>

static void swapFrames();

Mouse mouse {
    .sens = 0.1
};
Shader simpleShader;

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
}

void
loadVertices()
{
    D( glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, vCube) );
    D( glEnableVertexAttribArray(0) );
}

v3 eyePos {0, 0, 3};
v3 eyeF {0, 0, -1};
v3 eyeR {1, 0, 0};
const v3 up {0, 1, 0};

m4 proj = m4Pers(90.0f, (f32)appState.windowWidth / (f32)appState.windowHeight, 0.01f, 100.0f);
m4 view = m4LookAt(eyePos, eyePos + eyeF, up);

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

    D( glEnable(GL_DEPTH_TEST) );

    v4 gray = COLOR(0x121212ff);
    D( glClearColor(gray.r, gray.g, gray.b, gray.a) );

    loadVertices();
}

void
procMouse()
{
    auto offsetX = (mouse.prevX - mouse.lastX) * mouse.sens;
    auto offsetY = (mouse.lastY - mouse.prevY) * mouse.sens;

    mouse.prevX = mouse.lastX;
    mouse.prevY = mouse.lastY;

    mouse.yaw += offsetX;
    mouse.pitch += offsetY;

    if (mouse.pitch > 89.9f)
        mouse.pitch = 89.9f;
    if (mouse.pitch < -89.9f)
        mouse.pitch = -89.9f;

    eyeF = v3Norm({
        (f32)cos(TO_RAD(mouse.yaw)) * (f32)cos(TO_RAD(mouse.pitch)),
        (f32)sin(TO_RAD(mouse.pitch)),
        (f32)sin(TO_RAD(mouse.yaw)) * (f32)cos(TO_RAD(mouse.pitch))
    });

    eyeR = v3Norm(v3Cross(eyeF, up));

    LOG(OK, "pitch: {}\tyaw: {}\n", mouse.pitch, mouse.yaw);
}

void
drawFrame(void)
{
    if (!appState.paused)
    {

        D( glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) );

        m4 tm = m4Iden();

        procMouse();

        static f64 inc = 0;
        tm = m4RotX(tm, TO_RAD(inc));

        proj = m4Pers(90.0f, (f32)appState.windowWidth / (f32)appState.windowHeight, 0.01f, 100.0f);
        view = m4LookAt(eyePos, eyePos + eyeF, up);

        simpleShader.use();
        simpleShader.setMat4("proj", proj);
        simpleShader.setMat4("view", view);
        simpleShader.setMat4("model", tm);

        D( glDrawArrays(GL_TRIANGLES, 0, LEN(vCube)) );

        inc += 0.3;
    }
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
