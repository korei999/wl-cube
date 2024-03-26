#include "headers/frame.hh"
#include "headers/gmath.hh"
#include "headers/shader.hh"
#include "headers/utils.hh"
#include "headers/model.hh"
#include "headers/texture.hh"
#include "headers/wayland.hh"

PlayerControls player {
    .mouse.sens = 0.07,
    .pos {0.0, 0.0, 2.0},
    .moveSpeed = 4.0,
};

Shader lightSrc;
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
    lightSrc.loadShaders("shaders/simple.vert", "shaders/simple.frag");
    gouraud.loadShaders("shaders/gouraud.vert", "shaders/gouraud.frag");
}

static void
setupModels()
{
    hl.loadOBJ("test_assets/models/gordon/hl1.obj");
    cube.loadOBJ("test_assets/models/cube/cube.obj");

    bodyTex.loadBMP("test_assets/models/gordon/DM_Base.bmp");
    faceTex.loadBMP("test_assets/models/gordon/DM_Face.bmp");
}

void
WlClient::setupDraw()
{
    if (!eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext))
        LOG(FATAL, "eglMakeCurrent failed\n");

    EGLD( eglSwapInterval(eglDisplay, 1) );
    toggleFullscreen();

    D( glEnable(GL_CULL_FACE) );
    D( glEnable(GL_DEPTH_TEST) );
    D( glDepthFunc(GL_LESS) );

    v4 gray = COLOR(0x222222FF);
    D( glClearColor(gray.r, gray.g, gray.b, gray.a) );
}

f64 incCounter = 0;
f64 fov = 90.0f;

void
WlClient::drawFrame()
{
    player.updateDeltaTime();
    player.procMouse();
    player.procKeys();

    f32 aspect = (f32)wWidth / (f32)wHeight;

    if (!isPaused)
    {
        D( glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) );

        player.updateProj(TO_RAD(fov), aspect, 0.01f, 100.0f);
        player.updateView();
        m4 trm = m4Iden();

        v3 lightPos {2.0, 4.0, (f32)sin(incCounter) * 7};
        m4 lightTm = m4Iden();

        trm = m4Scale(trm, 0.05f);
        trm = m4Trans(trm, {0.5f, 0.5f, 0.5f});

        m3 normMat = m3Transpose(m3Inverse(trm));

        v3 lightColor {(sin(lightPos.x) + 1) / 2, 0.4, 0.7};
        v3 diffuseColor = lightColor * 0.8f;

        gouraud.use();
        gouraud.setM4("proj", player.proj);
        gouraud.setM4("view", player.view);
        gouraud.setM3("normMat", normMat);
        gouraud.setV3("viewPos", player.pos);

        gouraud.setV3("light.pos", lightPos);
        gouraud.setV3("light.ambient", ambLight);
        gouraud.setV3("light.diffuse", diffuseColor);
        gouraud.setF("light.constant", 1.0f);
        gouraud.setF("light.linear", 0.09f);
        gouraud.setF("light.quadratic", 0.032f);

        gouraud.setM4("model", trm);
        bodyTex.use();
        hl.draw(1);

        trm = m4RotY(trm, sin(incCounter) / 3);
        gouraud.setM4("model", trm);
        faceTex.use();
        hl.draw(0);

        lightTm = m4Trans(lightTm, lightPos);
        lightTm = m4Scale(lightTm, 0.05f);

        lightSrc.use();
        lightSrc.setM4("proj", player.proj);
        lightSrc.setM4("view", player.view);
        lightSrc.setM4("model", lightTm);
        lightSrc.setV3("lightColor", lightColor);
        cube.draw();

        incCounter += 1 * player.deltaTime;
    }
}

void 
WlClient::mainLoop()
{
    isRunning = true;
    isRelativeMode = true;
    isPaused = false;

    setupDraw();

    setupShaders();
    setupModels();

    while (isRunning)
    {
        drawFrame();

        if (!eglSwapBuffers(eglDisplay, eglSurface))
            LOG(FATAL, "eglSwapBuffers failed\n");

        if (wl_display_dispatch(display) == -1)
            LOG(FATAL, "wl_display_dispatch error\n");
    }
}
