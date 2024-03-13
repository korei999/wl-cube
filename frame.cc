#include "headers/frame.hh"
#include "headers/main.hh"
#include "headers/gmath.hh"
#include "headers/shader.hh"
#include "headers/utils.hh"
#include "headers/model.hh"

static void swapFrames();

PlayerControls player {
    .mouse.sens = 0.07,
    .pos {0.0, 0.0, 1.0},
    .moveSpeed = 1.0,
};

Shader simpleShader;
GLuint posBufferObj;
Model icosphere;
Model cube;

static void
setupShaders()
{
    simpleShader = {"shaders/sm.vert", "shaders/ht.frag"};
    simpleShader.queryActiveUniforms();
}


void
setupModels()
{
    icosphere = {"assets/models/icosphere/icosphere.obj"};
    cube = {"assets/models/cube/cube.obj"};
}

void
setupDraw()
{
    if (!eglMakeCurrent(appState.eglDisplay, appState.eglSurface, appState.eglSurface, appState.eglContext))
        LOG(FATAL, "eglMakeCurrent failed\n");

    EGLD( eglSwapInterval(appState.eglDisplay, 0) );

    setupShaders();
    appState.togglePointerRelativeMode();

    D( glEnable(GL_CULL_FACE) );
    D( glEnable(GL_DEPTH_TEST) );
    D( glDepthFunc(GL_LESS) );

    v4 gray = COLOR(0x121212ff);
    D( glClearColor(gray.r, gray.g, gray.b, gray.a) );

    setupModels();
}

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

        static f64 inc = 0;
        tm = m4Scale(tm, 0.5f);

        simpleShader.use();
        simpleShader.setM4("proj", player.proj);
        simpleShader.setM4("view", player.view);

        for (int i = 0; i < 20; i++)
        {
            tm = m4Trans(tm, {0, (f32)i, 0});
            tm = m4Rot(tm, TO_RAD(inc), v3Norm({0.25f, 0.50f, 1.00f}));

            simpleShader.setM4("model", tm);
            cube.drawMesh();
        }

        inc += 0.02;
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
