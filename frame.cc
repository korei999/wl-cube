#include "headers/frame.hh"
#include "headers/model.hh"

#include <thread>

#define SHADOW_WIDTH 2048
#define SHADOW_HEIGHT 2048

#ifdef DEBUG
static void
debugCallback(GLenum source,
              GLenum type,
              GLuint id,
              GLenum severity,
              GLsizei length,
              const GLchar* message,
              const void* user)
{
    int sev = LogSeverity::OK;
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:
            sev = LogSeverity::BAD;
            break;

        case GL_DEBUG_SEVERITY_MEDIUM:
            sev = LogSeverity::WARNING;
            break;

        case GL_DEBUG_SEVERITY_LOW:
            sev = LogSeverity::OK;
            break;

        default:
            break;
    }

    LOG(sev, "{}\n", message);
}
#endif

PlayerControls player {
    .pos {0.0, 1.0, 2.0},
    .moveSpeed = 4.0,
    .mouse {.sens = 0.07}
};

Shader debugDepthQuadSh;
Shader cubeDepthSh;
Shader omniDirShadowSh;
Shader colorSh;
Model cube;
Model sphere;
Model plane;
Model quad;
Model teaPot;
Model sponza;
Texture boxTex;
Texture dirtTex;
Ubo projView;
CubeMap cubeMap;

#ifdef FPS_COUNTER
f64 prevTime;
#endif

void
WlClient::setupDraw()
{
    this->bindGlContext();
    this->setSwapInterval(1);
    this->toggleFullscreen();

#ifdef DEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(debugCallback, this);
#endif
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    v4 gray = v4Color(0x444444FF);
    glClearColor(gray.r, gray.g, gray.b, gray.a);

    debugDepthQuadSh.loadShaders("shaders/shadows/debugQuad.vert", "shaders/shadows/debugQuad.frag");
    cubeDepthSh.loadShaders("shaders/shadows/cubeMap/cubeMapDepth.vert", "shaders/shadows/cubeMap/cubeMapDepth.geom", "shaders/shadows/cubeMap/cubeMapDepth.frag");
    omniDirShadowSh.loadShaders("shaders/shadows/cubeMap/omniDirShadow.vert", "shaders/shadows/cubeMap/omniDirShadow.frag");
    colorSh.loadShaders("shaders/simple.vert", "shaders/simple.frag");

    omniDirShadowSh.use();
    omniDirShadowSh.setI("uDiffuseTexture", 0);
    omniDirShadowSh.setI("uDepthMap", 1);

    debugDepthQuadSh.use();
    debugDepthQuadSh.setI("uDepthMap", 1);

    cubeMap = createCubeShadowMap(SHADOW_WIDTH, SHADOW_HEIGHT);

    projView.createBuffer(sizeof(m4) * 2, GL_DYNAMIC_DRAW);
    projView.bindBlock(&omniDirShadowSh, "ubProjView", 0);
    projView.bindBlock(&colorSh, "ubProjView", 0);

    quad = getQuad();

    /* TODO: find a better way for parallel asset loading */
    /* each thread cannot have the same opengl context */
    /* unbind before creating threads */
    this->unbindGlContext();
    /* models */
    std::thread m0(&Model::loadOBJ, &cube, "test-assets/models/cube/cube.obj", GL_STATIC_DRAW, this);
    std::thread m1(&Model::loadOBJ, &sponza, "test-assets/models/Sponza/sponza.obj", GL_STATIC_DRAW, this);
    std::thread m2(&Model::loadOBJ, &sphere, "test-assets/models/icosphere/icosphere.obj", GL_STATIC_DRAW, this);
    /* textures */
    // std::thread t0(&Texture::loadBMP, &boxTex, "test-assets/silverBox.bmp", false, GL_MIRRORED_REPEAT, this);
    // std::thread t1(&Texture::loadBMP, &dirtTex, "test-assets/dirt.bmp", false, GL_MIRRORED_REPEAT, this);

    m0.join();
    m1.join();
    m2.join();
    // t0.join();
    // t1.join();

    /* restore context after assets are loaded */
    this->bindGlContext();
}

f64 incCounter = 0;
f64 fov = 90.0f;
bool showFb = false;
f32 x = 0, y = 0, z = 0;

void
renderScene(Shader* sh, bool depth)
{
    m4 m = m4Iden();

    m = m4Scale(m, 0.01);
    sh->setM4("uModel", m);
    if (!depth)
    {
        sh->setM3("uNormalMatrix", m3Normal(m));
        boxTex.bind(GL_TEXTURE0);
    }
    sponza.drawTex();
}

void
WlClient::drawFrame()
{
#ifdef FPS_COUNTER
    static int fpsCount = 0;
    f64 currTime = timeNow();
    if (currTime >= prevTime + 1.0)
    {
        CERR("fps: {}, ms: {:.3f}\n", fpsCount, player.deltaTime);
        fpsCount = 0;
        prevTime = currTime;
    }
#endif

    player.updateDeltaTime();
    player.procMouse();
    player.procKeys(this);

    f32 aspect = (f32)wWidth / (f32)wHeight;
    constexpr f32 shadowAspect = (f32)SHADOW_WIDTH / (f32)SHADOW_HEIGHT;

    if (!isPaused)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        player.updateProj(toRad(fov), aspect, 0.01f, 100.0f);
        player.updateView();
        /* copy both proj and view in one go */
        projView.bufferData(&player, 0, sizeof(m4) * 2);

        // v3 lightPos {x, 4, -1};
        v3 lightPos {(f32)sin(player.currTime) * 7, 2, 0};
        f32 nearPlane = 0.01, farPlane = 25.0;
        m4 shadowProj = m4Pers(toRad(90), shadowAspect, nearPlane, farPlane);
        CubeMapProjections shadowTms(shadowProj, lightPos);

        /* render scene to depth cubemap */
        glViewport(0, 0, cubeMap.width, cubeMap.height);
        glBindFramebuffer(GL_FRAMEBUFFER, cubeMap.fbo);
        glClear(GL_DEPTH_BUFFER_BIT);

        cubeDepthSh.use();
        constexpr auto len = LEN(shadowTms);
        for (size_t i = 0; i < len; i++)
            cubeDepthSh.setM4("uShadowMatrices[" + std::to_string(i) + "]", shadowTms[i]);
        cubeDepthSh.setV3("uLightPos", lightPos);
        cubeDepthSh.setF("uFarPlane", farPlane);
        glActiveTexture(GL_TEXTURE1);
        glCullFace(GL_FRONT);
        renderScene(&cubeDepthSh, true);
        glCullFace(GL_BACK);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        /* reset viewport */
        glViewport(0, 0, this->wWidth, this->wHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /* render scene as normal using the denerated depth map */
        omniDirShadowSh.use();
        omniDirShadowSh.setV3("uLightPos", lightPos);
        omniDirShadowSh.setV3("uViewPos", player.pos);
        omniDirShadowSh.setF("uFarPlane", farPlane);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap.tex);
        renderScene(&omniDirShadowSh, false);

        /* draw light source */
        m4 cubeTm = m4Iden();
        cubeTm = m4Translate(cubeTm, lightPos);
        cubeTm = m4Scale(cubeTm, 0.05);
        colorSh.use();
        colorSh.setM4("uModel", cubeTm);
        colorSh.setV3("uColor", v3(1, 1, 1));
        sphere.drawTex();

        incCounter += 1.0 * player.deltaTime;
    }
#ifdef FPS_COUNTER
        fpsCount++;
#endif
}

void 
WlClient::mainLoop()
{
    isRunning = true;
    isRelativeMode = true;
    isPaused = false;

    setupDraw();

#ifdef FPS_COUNTER
    prevTime = timeNow();
#endif

    while (isRunning)
    {
        drawFrame();

        EGLD( eglSwapBuffers(eglDisplay, eglSurface) );
        if (wl_display_dispatch(display) == -1)
            LOG(FATAL, "wl_display_dispatch error\n");
    }
}
