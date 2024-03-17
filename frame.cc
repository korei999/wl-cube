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
Shader gouraud;
Model hl;
Model cube;
u32 tex;
Texture bodyTex;
Texture faceTex;
v3 ambLight {0.2, 0.2, 0.2};

static void
setupShaders()
{
    spotLight.loadShaders("shaders/simple.vert", "shaders/simple.frag");
    gouraud.loadShaders("shaders/gouraud.vert", "shaders/gouraud.frag");
    gouraud.queryActiveUniforms();
}

void
setupModels()
{
    hl.loadOBJ("test_assets/models/gordon/hl1.obj");
    cube.loadOBJ("test_assets/models/cube/cube.obj");

    bodyTex.loadBMP("test_assets/models/gordon/DM_Base.bmp");
    faceTex.loadBMP("test_assets/models/gordon/DM_Face.bmp");
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
        m4 trm = m4Iden();

        v3 lightPos {(f32)sin(incCounter) * 2, 4.0, 2.0};
        m4 lightTm = m4Iden();

        trm = m4Scale(trm, 0.05f);
        trm = m4Trans(trm, {0.5f, 0.5f, 0.5f});

        m3 normMat = m3Transpose(m3Inverse(trm));

        v3 lightColor {(sin(lightPos.x) + 1) / 2, 0.4, 0.7};

        gouraud.use();
        gouraud.setM4("proj", player.proj);
        gouraud.setM4("view", player.view);
        gouraud.setV3("lightPos", lightPos);
        gouraud.setV3("ambLight", ambLight);
        gouraud.setV3("lightColor", lightColor);
        gouraud.setM3("normMat", normMat);
        gouraud.setV3("viewPos", player.pos);

        gouraud.setM4("model", trm);
        bodyTex.use();
        hl.draw(1);

        trm = m4RotY(trm, sin(incCounter) / 3);
        gouraud.setM4("model", trm);
        faceTex.use();
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
