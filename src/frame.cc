#include <cmath>
#include <thread>

#include "frame.hh"
#include "colors.hh"
#include "model.hh"
#include "threadpool.hh"

#define SHADOW_WIDTH 1024
#define SHADOW_HEIGHT 1024

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
    /*int sev = LogSeverity::OK;*/
    const char* typeStr {};
    const char* sourceStr {};

    switch (source)
    {
        case GL_DEBUG_SOURCE_API: sourceStr = "API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM: sourceStr = "Window System"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: sourceStr = "Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY: sourceStr = "Third Party"; break;
        case GL_DEBUG_SOURCE_APPLICATION: sourceStr = "Application"; break;
        case GL_DEBUG_SOURCE_OTHER: sourceStr = "Other"; break;

        default: break;
    }

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR: typeStr = "Error"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeStr = "Deprecated Behavior"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: typeStr = "Undefined Behavior"; break;
        case GL_DEBUG_TYPE_PORTABILITY: typeStr = "Portability"; break;
        case GL_DEBUG_TYPE_PERFORMANCE: typeStr = "Performance"; break;
        case GL_DEBUG_TYPE_MARKER: typeStr = "Marker"; break;
        case GL_DEBUG_TYPE_PUSH_GROUP: typeStr = "Push Group"; break;
        case GL_DEBUG_TYPE_POP_GROUP: typeStr = "Pop Group"; break;
        case GL_DEBUG_TYPE_OTHER: typeStr = "Other"; break;

        default: break;
    }

    /*switch (severity)*/
    /*{*/
    /*    case GL_DEBUG_SEVERITY_HIGH: sev = LogSeverity::BAD; break;*/
    /*    case GL_DEBUG_SEVERITY_MEDIUM: sev = LogSeverity::WARNING; break;*/
    /**/
    /*    case GL_DEBUG_SEVERITY_LOW:*/
    /*    default: break;*/
    /*}*/

    CERR("source: '{}', type: '{}'\n{}\n", sourceStr, typeStr, message);
}

#endif

PlayerControls player {
    .pos {0.0, 1.0, 1.0},
    .moveSpeed = 4.0,
    .mouse {.sens = 0.07}
};

Shader shDebugDepthQuad;
Shader shCubeDepth;
Shader shOmniDirShadow;
Shader shColor;
Shader shTex;
Shader shBF;
Shader shNormalMapping;
Model mSphere;
Model mSponza;
Model mCar;
Texture mBoxTex;
Texture mDirtTex;
Ubo uboProjView;
CubeMap cmCubeMap;

#ifdef FPS_COUNTER
f64 _prevTime;
#endif

void
prepareDraw(App* app)
{
    app->bindGlContext();
    app->showWindow();
    app->setSwapInterval(1);
    app->toggleFullscreen();

#ifdef DEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(debugCallback, app);
#endif
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);

    v4 gray = v4Color(0x444444FF);
    glClearColor(gray.r, gray.g, gray.b, gray.a);

    shDebugDepthQuad.loadShaders("shaders/shadows/debugQuad.vert", "shaders/shadows/debugQuad.frag");
    shCubeDepth.loadShaders("shaders/shadows/cubeMap/cubeMapDepth.vert", "shaders/shadows/cubeMap/cubeMapDepth.geom", "shaders/shadows/cubeMap/cubeMapDepth.frag");
    shOmniDirShadow.loadShaders("shaders/shadows/cubeMap/omniDirShadow.vert", "shaders/shadows/cubeMap/omniDirShadow.frag");
    shColor.loadShaders("shaders/simple.vert", "shaders/simple.frag");
    shTex.loadShaders("shaders/simpleTex.vert", "shaders/simpleTex.frag");
    shNormalMapping.loadShaders("shaders/normalMapping.vert", "shaders/normalMapping.frag");

    shOmniDirShadow.use();
    shOmniDirShadow.setI("uDiffuseTexture", 0);
    shOmniDirShadow.setI("uDepthMap", 1);

    shNormalMapping.use();
    shNormalMapping.setI("uDiffuseTex", 0);
    shNormalMapping.setI("uNormalMap", 1);

    shDebugDepthQuad.use();
    shDebugDepthQuad.setI("uDepthMap", 1);

    cmCubeMap = createCubeShadowMap(SHADOW_WIDTH, SHADOW_HEIGHT);

    uboProjView.createBuffer(sizeof(m4) * 2, GL_DYNAMIC_DRAW);
    uboProjView.bindBlock(&shOmniDirShadow, "ubProjView", 0);
    uboProjView.bindBlock(&shColor, "ubProjView", 0);
    uboProjView.bindBlock(&shTex, "ubProjView", 0);
    uboProjView.bindBlock(&shNormalMapping, "ubProjView", 0);

    /* unbind before creating threads */
    app->unbindGlContext();

    ThreadPool tp(std::thread::hardware_concurrency());

    tp.submit([&]{ mSphere.load("test-assets/models/icosphere/gltf/untitled.gltf", GL_STATIC_DRAW, GL_MIRRORED_REPEAT, app); });
    tp.submit([&]{ mSponza.load("test-assets/models/Sponza/Sponza.gltf", GL_STATIC_DRAW, GL_MIRRORED_REPEAT, app); });
    tp.submit([&]{ mCar.load("test-assets/models/backpack/scene.gltf", GL_STATIC_DRAW, GL_MIRRORED_REPEAT, app); });
    tp.wait();

    /* restore context after assets are loaded */
    app->bindGlContext();
}

f64 incCounter = 0;
f32 fov = 90.0f;
f64 x = 0.0, y = 0.0, z = 0.0;

static void
renderScene(Shader* sh, bool depth)
{
    m4 m = m4Iden();
    if (!depth) sh->setM3("uNormalMatrix", m3Normal(m));
    mSponza.draw(DRAW::DIFF_TEX | DRAW::APPLY_TM, sh, "uModel", m);

    /*static f32 i = 0.0f;*/
    /**/
    /*m = m4Translate(m4Iden(), {-0.4, 0.6, 0});*/
    /*qt q = qtAxisAngle({1, 0, 0}, toRad(-90.0f)) * qtAxisAngle({0, 1, 0}, i);*/
    /*q *= qtAxisAngle({0, 1, 0}, i);*/
    /*m *= qtRot(q);*/
    /*m = m4Scale(m, 1.0f);*/
    /*i += 0.130f * player.deltaTime;*/
    /**/
    /*if (!depth) sh->setM3("uNormalMatrix", m3Normal(m));*/
    /*glDisable(GL_CULL_FACE);*/
    /*mCar.draw(DRAW::DIFF_TEX | DRAW::APPLY_TM, sh, "uModel", m);*/
    /*glEnable(GL_CULL_FACE);*/
}

static void
drawFrame(App* app)
{
    player.updateDeltaTime();
    player.procMouse();
    player.procKeys(app);

    f32 aspect = static_cast<f32>(app->wWidth) / static_cast<f32>(app->wHeight);
    constexpr f32 shadowAspect = static_cast<f32>(SHADOW_WIDTH) / static_cast<f32>(SHADOW_HEIGHT);

    if (!app->bPaused)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        player.updateProj(toRad(fov), aspect, 0.01f, 100.0f);
        player.updateView();
        /* copy both proj and view in one go */
        uboProjView.bufferData(&player, 0, sizeof(m4) * 2);

        // v3 lightPos {x, 4, -1};
        v3 lightPos {std::cos(player.currTime) * 6.0f, 3, std::sin(player.currTime) * 1.1f};
        constexpr v3 lightColor(Color::snow);
        f32 nearPlane = 0.01f, farPlane = 25.0f;
        m4 shadowProj = m4Pers(toRad(90), shadowAspect, nearPlane, farPlane);
        CubeMapProjections shadowTms(shadowProj, lightPos);

        /* render scene to depth cubemap */
        glViewport(0, 0, cmCubeMap.width, cmCubeMap.height);
        glBindFramebuffer(GL_FRAMEBUFFER, cmCubeMap.fbo);
        glClear(GL_DEPTH_BUFFER_BIT);

        shCubeDepth.use();
        constexpr auto len = std::size(shadowTms.tms);
        for (size_t i = 0; i < len; i++)
            shCubeDepth.setM4(FMT("uShadowMatrices[{}]", i), shadowTms[i]);
        shCubeDepth.setV3("uLightPos", lightPos);
        shCubeDepth.setF("uFarPlane", farPlane);
        glActiveTexture(GL_TEXTURE1);
        glCullFace(GL_FRONT);
        renderScene(&shCubeDepth, true);
        glCullFace(GL_BACK);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        /* reset viewport */
        glViewport(0, 0, app->wWidth, app->wHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /*render scene as normal using the denerated depth map */
        shOmniDirShadow.use();
        shOmniDirShadow.setV3("uLightPos", lightPos);
        shOmniDirShadow.setV3("uLightColor", lightColor);
        shOmniDirShadow.setV3("uViewPos", player.pos);
        shOmniDirShadow.setF("uFarPlane", farPlane);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cmCubeMap.tex);
        renderScene(&shOmniDirShadow, false);

        /* draw light source */
        m4 tmCube = m4Iden();
        tmCube = m4Translate(tmCube, lightPos);
        tmCube = m4Scale(tmCube, 0.05f);
        shColor.use();
        shColor.setV3("uColor", lightColor);
        mSphere.draw(DRAW::APPLY_TM, &shColor, "uModel", tmCube);

        incCounter += 1.0 * player.deltaTime;
    }
}

void
run(App* app)
{
    app->bRunning = true;
    app->bRelativeMode = true;
    app->bPaused = false;
    app->setCursorImage("default");

#ifdef FPS_COUNTER
    _prevTime = timeNow();
#endif

    prepareDraw(app);
    player.updateDeltaTime(); /* reset delta time before drawing */
    player.updateDeltaTime();

    while (app->bRunning)
    {
#ifdef FPS_COUNTER
    static int _fpsCount = 0;
    f64 _currTime = timeNow();
    if (_currTime >= _prevTime + 1.0)
    {
        CERR("fps: {}, ms: {:.3f}\n", _fpsCount, player.deltaTime);
        _fpsCount = 0;
        _prevTime = _currTime;
    }
#endif
    /* drawing */
        app->procEvents();

        drawFrame(app);

        app->swapBuffers();
    /* drawing */
#ifdef FPS_COUNTER
        _fpsCount++;
#endif
    }
}
