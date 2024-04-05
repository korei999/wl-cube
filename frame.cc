#include "headers/frame.hh"
#include "headers/gmath.hh"
#include "headers/shader.hh"
#include "headers/utils.hh"
#include "headers/model.hh"
#include "headers/texture.hh"
#include "headers/wayland.hh"

#define SHADOW_WIDTH 1024
#define SHADOW_HEIGHT 1024

struct ShadowMap
{
    GLuint fbo;
    GLuint tex;
    int width;
    int height;
};

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
            sev = LogSeverity::FATAL;
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
    .mouse.sens = 0.07,
};

Shader shadowSh;
Shader depthSh;
Shader debugDepthQuadSh;
Model cube;
Model plane;
Model quad;
Model teaPot;
Texture boxTex;
Texture dirtTex;
v3 ambLight = COLOR3(0x444444);
Ubo projView;
ShadowMap depthMap;

#ifdef FPS_COUNTER
f64 prevTime;
#endif

ShadowMap
createShadowMap(const int width, const int height)
{
    GLenum none = GL_NONE;
    ShadowMap res {};
    res.width = width;
    res.height = height;

    D( glGenTextures(1, &res.tex) );
    D( glBindTexture(GL_TEXTURE_2D, res.tex) );
    D( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR) );
    D( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR) );
    D( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER) );
    D( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER) );
    D( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE) );
    D( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL) );
	f32 borderColor[] {1.0, 1.0, 1.0, 1.0};
	D( glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor) );
    D( glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, nullptr) );
    D( glBindTexture(GL_TEXTURE_2D, 0) );

    GLint defFramebuffer = 0;
    D( glGetIntegerv(GL_FRAMEBUFFER_BINDING, &defFramebuffer) );
    /* set up fbo */
    D( glGenFramebuffers(1, &res.fbo) );
    D( glBindFramebuffer(GL_FRAMEBUFFER, res.fbo) );

    D( glDrawBuffers(1, &none) );
    D( glReadBuffer(GL_NONE) );

    D( glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, res.tex, 0) );

    D( glActiveTexture(GL_TEXTURE0) );
    D( glBindTexture(GL_TEXTURE_2D, res.tex) );

    if (GL_FRAMEBUFFER_COMPLETE != glCheckFramebufferStatus(GL_FRAMEBUFFER))
        LOG(FATAL, "glCheckFramebufferStatus != GL_FRAMEBUFFER_COMPLETE\n"); 

    D( glBindFramebuffer(GL_FRAMEBUFFER, 0) );

    return res;
}

void
WlClient::setupDraw()
{
    if (!eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext))
        LOG(FATAL, "eglMakeCurrent failed\n");

    EGLD( eglSwapInterval(eglDisplay, swapInterval) );
    toggleFullscreen();

#ifdef DEBUG
    D( glEnable(GL_DEBUG_OUTPUT) );
    D( glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS) );
    D( glDebugMessageCallback(debugCallback, this) );
#endif

    D( glEnable(GL_CULL_FACE) );
    D( glEnable(GL_DEPTH_TEST) );

    v4 gray = COLOR4(0x222222FF);
    D( glClearColor(gray.r, gray.g, gray.b, gray.a) );

    shadowSh.loadShaders("shaders/shadows/shadows.vert", "shaders/shadows/shadows.frag");
    depthSh.loadShaders("shaders/shadows/depth.vert", "shaders/shadows/depth.frag");
    debugDepthQuadSh.loadShaders("shaders/shadows/debugQuad.vert", "shaders/shadows/debugQuad.frag");

    cube.loadOBJ("test-assets/models/cube/cube.obj");
    plane.loadOBJ("test-assets/models/plane/plane.obj");
    teaPot.loadOBJ("test-assets/models/teapot/teapot.obj");
    quad = getQuad();

    boxTex.loadBMP("test-assets/silverBox.bmp");
    dirtTex.loadBMP("test-assets/dirt.bmp");

    depthMap = createShadowMap(SHADOW_WIDTH, SHADOW_HEIGHT);
    shadowSh.use();
    shadowSh.setI("uDiffuseTexture", 0);
    shadowSh.setI("uShadowMap", 1);
    debugDepthQuadSh.use();
    debugDepthQuadSh.setI("uDepthMap", 0);

    projView.createBuffer(sizeof(m4) * 2, GL_DYNAMIC_DRAW);
    projView.bindBlock(&shadowSh, "ubProjView", 0);
}

f64 incCounter = 0;
f64 fov = 90.0f;
bool showFb = false;
f32 x = 0, y = 0, z = 0;

void
renderScene(Shader* sh, bool depth)
{
    m4 m = m4Scale(m4Translate(m4Iden(), {0, 0, 0}), 10);
    sh->use();
    sh->setM4("uModel", m);
    if (!depth)
    {
        sh->setM3("uNormalMatrix", m3Normal(m));
        dirtTex.bind(GL_TEXTURE0);
    }
    plane.draw();

    m = m4Scale(m4Translate(m4Iden(), {1, 0.2, 0}), 0.2);
    sh->setM4("uModel", m);
    if (!depth)
    {
        sh->setM3("uNormalMatrix", m3Normal(m));
        dirtTex.bind(GL_TEXTURE0);
    }
    teaPot.draw();

    m = m4Scale(m4Translate(m4Iden(), {0.1, 0.2, 1}), 0.2);
    sh->setM4("uModel", m);
    if (!depth)
    {
        sh->setM3("uNormalMatrix", m3Normal(m));
        boxTex.bind(GL_TEXTURE0);
    }
    cube.draw();

    m = m4Translate(m4Iden(), {1, 2, 0.5});
    v3 axis = v3Norm({0, -3, 1});
    m = m4Scale(m4Rot(m, TO_RAD(60), axis), 0.1);
    sh->setM4("uModel", m);
    if (!depth)
    {
        sh->setM3("uNormalMatrix", m3Normal(m));
        dirtTex.bind(GL_TEXTURE0);
    }
    teaPot.draw();
}

void
WlClient::drawFrame()
{
#ifdef FPS_COUNTER
    static int fpsCount = 0;
    f64 currTime = timeNow();
    if (currTime >= prevTime + 1.0)
    {
        std::print(stderr, "fps: {}, ms: {:.3f}\n", fpsCount, player.deltaTime);
        fpsCount = 0;
        prevTime = currTime;
    }
#endif

    player.updateDeltaTime();
    player.procMouse();
    player.procKeys(this);

    f32 aspect = (f32)wWidth / (f32)wHeight;

    if (!isPaused)
    {
        D( glClearColor(0.4, 0.4, 0.4, 1.0) );
        D( glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) );

        player.updateProj(TO_RAD(fov), aspect, 0.01f, 100.0f);
        player.updateView();
        /* copy both proj and view in one go */
        projView.bufferData(&player, 0, sizeof(m4) * 2);

        // v3 lightPos {x, 4, -1};
        v3 lightPos {(f32)sin(player.currTime) * 5, 4, -1 };
        f32 nearPlane = 1.0, farPlane = 20.0;
        // m4 lightProj = m4Ortho(-10, 10, -10, 10, nearPlane, farPlane);
        m4 lightProj = m4Pers(TO_RAD(90), aspect, nearPlane, farPlane );
        m4 lightView = m4LookAt(lightPos, {0, 0, 0}, player.up);
        m4 lightSpaceMatrix = lightProj * lightView;

        /* render scene from light's point of view */
        depthSh.use();
        depthSh.setM4("uLightSpaceMatrix", lightSpaceMatrix);

        D( glViewport(0, 0, depthMap.width, depthMap.height) );
        D( glBindFramebuffer(GL_FRAMEBUFFER, depthMap.fbo) );
        D( glClear(GL_DEPTH_BUFFER_BIT) );
        D( glCullFace(GL_FRONT) );
        renderScene(&depthSh, true);
        D( glCullFace(GL_BACK) );
        D( glBindFramebuffer(GL_FRAMEBUFFER, 0) );

        /* reset viewport */
        D( glViewport(0, 0, this->wWidth, this->wHeight) );
        D( glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) );

        /* render scene as normal using the denerated depth map */
        shadowSh.use();
        shadowSh.setM4("uProj", player.proj);
        shadowSh.setM4("uView", player.view);
        /* set light uniforms */
        shadowSh.setV3("uViewPos", player.pos);
        shadowSh.setV3("uLightPos", lightPos);
        shadowSh.setM4("uLightSpaceMatrix", lightSpaceMatrix);
        D( glActiveTexture(GL_TEXTURE1) );
        D( glBindTexture(GL_TEXTURE_2D, depthMap.tex) );

        renderScene(&shadowSh, false);

        if (showFb) /* KEY_B */
        {
            debugDepthQuadSh.use();
            debugDepthQuadSh.setF("uNearPlane", nearPlane);
            debugDepthQuadSh.setF("uFarPlane", farPlane);
            D( glActiveTexture(GL_TEXTURE0) );
            D( glBindTexture(GL_TEXTURE_2D, depthMap.tex) );
            drawQuad(quad);
        }

        incCounter += 1 * player.deltaTime;
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
    swapInterval = 1;

    setupDraw();

#ifdef FPS_COUNTER
    prevTime = timeNow();
#endif

    while (isRunning)
    {
        drawFrame();

        swapBuffersAndDispatch();
    }

    D( glDeleteFramebuffers(1, &depthMap.fbo) );
    D( glDeleteTextures(1, &depthMap.tex) );
}

void
WlClient::swapBuffersAndDispatch()
{
    if (!eglSwapBuffers(eglDisplay, eglSurface))
        LOG(FATAL, "eglSwapBuffers failed\n");
    if (wl_display_dispatch(display) == -1)
        LOG(FATAL, "wl_display_dispatch error\n");
}
