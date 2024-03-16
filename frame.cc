#include "headers/frame.hh"
#include "headers/main.hh"
#include "headers/gmath.hh"
#include "headers/shader.hh"
#include "headers/utils.hh"
#include "headers/model.hh"
#include "headers/texture.hh"

PlayerControls player {
    .mouse.sens = 0.07,
    .pos {0.0, 0.0, 1.0},
    .moveSpeed = 1.0,
};

Shader simpleShader;
Shader lightShader;
GLuint posBufferObj;
Model hl;
Model cube;
u32 tex;
Texture body;
Texture face;

static void
setupShaders()
{
    simpleShader.loadShaders("shaders/sm.vert", "shaders/ht.frag");
    lightShader.loadShaders("shaders/light.vert", "shaders/light.frag");
}

void
setupModels()
{
    hl.loadOBJ("test_assets/models/gordon/hl1.obj");
    cube.loadOBJ("test_assets/models/cube/cube.obj");

    body.loadBMP("test_assets/models/gordon/DM_Base.bmp");
    face.loadBMP("test_assets/models/gordon/DM_Face.bmp");
}

void
setupDraw()
{
    if (!eglMakeCurrent(appState.eglDisplay, appState.eglSurface, appState.eglSurface, appState.eglContext))
        LOG(FATAL, "eglMakeCurrent failed\n");

    EGLD( eglSwapInterval(appState.eglDisplay, 0) );

    D( glEnable(GL_CULL_FACE) );
    D( glEnable(GL_DEPTH_TEST) );
    D( glDepthFunc(GL_LESS) );

    v4 gray = COLOR(0x333333ff);
    D( glClearColor(gray.r, gray.g, gray.b, gray.a) );

    appState.togglePointerRelativeMode();

    setupShaders();
    setupModels();
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

        v3 lightPos {(f32)sin(incCounter) * 2, 2.0, -3.0};
        m4 lightTm = m4Iden();

        tm = m4Scale(tm, 0.005f);
        tm = m4Trans(tm, {0.5f, 0.5f, 0.5f});

        lightShader.use();
        lightShader.setM4("proj", player.proj);
        lightShader.setM4("view", player.view);
        lightShader.setV3("lightPos", lightPos);

        lightShader.setM4("model", tm);
        body.use();
        hl.draw(1);

        tm = m4RotY(tm, sin(incCounter) / 3);
        lightShader.setM4("model", tm);
        face.use();
        hl.draw(0);

        lightTm = m4Trans(lightTm, lightPos);
        lightTm = m4Scale(lightTm, 0.05f);

        simpleShader.use();
        simpleShader.setM4("proj", player.proj);
        simpleShader.setM4("view", player.view);
        simpleShader.setM4("model", lightTm);
        cube.draw();

        incCounter += 1 * player.deltaTime;
    }

    swapFrames();
}
