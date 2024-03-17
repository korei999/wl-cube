#include "headers/frame.hh"
#include "headers/main.hh"
#include "headers/gmath.hh"
#include "headers/shader.hh"
#include "headers/utils.hh"
#include "headers/model.hh"
#include "headers/texture.hh"

PlayerControls player {
    .mouse.sens = 0.07,
    .pos {0.0, 0.0, 2.0},
    .moveSpeed = 4.0,
};

Shader spotLight;
Shader phong;
Model hl;
Model cube;
u32 tex;
Texture body;
Texture face;
v3 ambLight {0.2, 0.2, 0.2};
v3 lightColor {1.0, 1.0, 0.7};

static void
setupShaders()
{
    spotLight.loadShaders("shaders/simple.vert", "shaders/simple.frag");
    phong.loadShaders("shaders/phong.vert", "shaders/phong.frag");
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

        v3 lightPos {(f32)sin(incCounter) * 2, 4.0, 2.0};
        m4 lightTm = m4Iden();

        tm = m4Scale(tm, 0.05f);
        // tm = m4Scale(tm, v3Norm({3.5, 1.0, 1.0}));
        tm = m4Trans(tm, {0.5f, 0.5f, 0.5f});

        m3 normMat = m3Transpose(m3Inverse(tm));

        phong.use();
        phong.setM4("proj", player.proj);
        phong.setM4("view", player.view);
        phong.setV3("lightPos", lightPos);
        phong.setV3("ambLight", ambLight);
        phong.setV3("lightColor", lightColor);
        phong.setM3("normMat", normMat);

        phong.setM4("model", tm);
        body.use();
        hl.draw(1);

        tm = m4RotY(tm, sin(incCounter) / 3);
        phong.setM4("model", tm);
        face.use();
        hl.draw(0);

        lightTm = m4Trans(lightTm, lightPos);
        lightTm = m4Scale(lightTm, 0.05f);

        spotLight.use();
        spotLight.setM4("proj", player.proj);
        spotLight.setM4("view", player.view);
        spotLight.setM4("model", lightTm);
        spotLight.setV3("lightColor", lightColor);
        cube.draw();

        incCounter += 1 * player.deltaTime;
    }

    swapFrames();
}
