#include "headers/frame.hh"
#include "headers/main.hh"
#include "headers/gmath.hh"
#include "headers/shader.hh"
#include "headers/utils.hh"
#include "headers/model.hh"

#include <random>

static void swapFrames();

PlayerControls player {
    .mouse.sens = 0.07,
    .pos {0.0, 0.0, 1.0},
    .moveSpeed = 1.0,
};

Shader simpleShader;
GLuint posBufferObj;
Model backpack;

static void
setupShaders()
{
    simpleShader = {"shaders/ht.vert", "shaders/ht.frag"};
    simpleShader.queryActiveUniforms();
}


void
setupModels()
{
    backpack = {"assets/models/backpack/backpack.obj"};
    // backpack = {"assets/models/cube/cube.obj"};
}

void
setupDraw()
{
    if (!eglMakeCurrent(appState.eglDisplay, appState.eglSurface, appState.eglSurface, appState.eglContext))
        LOG(FATAL, "eglMakeCurrent failed\n");

    EGLD( eglSwapInterval(appState.eglDisplay, 0) );

    appState.togglePointerRelativeMode();
    setupShaders();
    setupModels();

    D( glEnable(GL_CULL_FACE) );
    D( glEnable(GL_DEPTH_TEST) );
    D( glDepthFunc(GL_LESS) );

    v4 gray = COLOR(0x121212ff);
    D( glClearColor(gray.r, gray.g, gray.b, gray.a) );
}


f64 incCounter = 0;

void
drawFrame(void)
{
    player.updateDeltaTime();
    player.procMouse();
    player.procKeys();

    f32 aspect = (f32)appState.wWidth / (f32)appState.wHeight;

    if (!appState.paused)
    {
        D( glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) );

        player.updateProj(90.0f, aspect, 0.01f, 100.0f);
        player.updateView();
        m4 tm = m4Iden();

        tm = m4Scale(tm, 0.1f);
        tm = m4Trans(tm, {0.5f, 0.5f, 0.5f});

        simpleShader.use();
        simpleShader.setM4("proj", player.proj);
        simpleShader.setM4("view", player.view);

        for (size_t i = 0; i < backpack.meshes.size(); i++)
        {
            auto time = sin(getTimeNow());
            if (EVEN(i))
                tm = m4Trans(tm, v3(time / 5, 0, 0));
            else if (i % 3 == 0)
                tm = m4Trans(tm, v3(0, time / 5, 0));
            else
                tm = m4Trans(tm, v3(0, 0, time / 5));

            simpleShader.setM4("model", tm);
            backpack.draw(i);
        }
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
