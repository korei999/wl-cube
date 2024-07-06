#include "frame.hh"
#include "colors.hh"
#include "model.hh"

#include <cmath>
#include <thread>

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

Shader debugDepthQuadSh;
Shader cubeDepthSh;
Shader omniDirShadowSh;
Shader colorSh;
Shader texSh;
Shader shBF;
Model cube;
Model sphere;
Model plane;
Model teaPot;
Model sponza;
Model duck;
Texture boxTex;
Texture dirtTex;
Texture duckTex;
Ubo projView;
CubeMap cubeMap;

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

    debugDepthQuadSh.loadShaders("shaders/shadows/debugQuad.vert", "shaders/shadows/debugQuad.frag");
    cubeDepthSh.loadShaders("shaders/shadows/cubeMap/cubeMapDepth.vert", "shaders/shadows/cubeMap/cubeMapDepth.geom", "shaders/shadows/cubeMap/cubeMapDepth.frag");
    omniDirShadowSh.loadShaders("shaders/shadows/cubeMap/omniDirShadow.vert", "shaders/shadows/cubeMap/omniDirShadow.frag");
    colorSh.loadShaders("shaders/simple.vert", "shaders/simple.frag");
    texSh.loadShaders("shaders/simpleTex.vert", "shaders/simpleTex.frag");

    omniDirShadowSh.use();
    omniDirShadowSh.setI("uDiffuseTexture", 0);
    omniDirShadowSh.setI("uDepthMap", 1);

    debugDepthQuadSh.use();
    debugDepthQuadSh.setI("uDepthMap", 1);

    cubeMap = createCubeShadowMap(SHADOW_WIDTH, SHADOW_HEIGHT);

    projView.createBuffer(sizeof(m4) * 2, GL_DYNAMIC_DRAW);
    projView.bindBlock(&omniDirShadowSh, "ubProjView", 0);
    projView.bindBlock(&colorSh, "ubProjView", 0);
    projView.bindBlock(&texSh, "ubProjView", 0);

    /* unbind before creating threads */
    app->unbindGlContext();
    /* models */
    {
        std::jthread m0(&Model::loadOBJ, &cube, "test-assets/models/cube/cube.obj", GL_STATIC_DRAW, GL_MIRRORED_REPEAT, app);
        /*std::jthread m1(&Model::loadOBJ, &sponza, "test-assets/models/Sponza/sponza.obj", GL_STATIC_DRAW, GL_MIRRORED_REPEAT, app);*/
        std::jthread m2(&Model::loadOBJ, &teaPot, "test-assets/models/teapot/teapot.obj", GL_STATIC_DRAW, GL_MIRRORED_REPEAT, app);
        /*std::jthread m2(&Model::loadOBJ, &sphere, "test-assets/models/icosphere/icosphere.obj", GL_STATIC_DRAW, GL_MIRRORED_REPEAT, app);*/
        std::jthread m3([&]{ sphere.loadOBJ("test-assets/models/icosphere/icosphere.obj", GL_STATIC_DRAW, GL_MIRRORED_REPEAT, app); });
        /*std::jthread m4([&]{ duck.loadGLTF("test-assets/models/duck/Duck.gltf", GL_STATIC_DRAW, GL_MIRRORED_REPEAT, app); });*/
        /*std::jthread m4([&]{ duck.loadGLTF("test-assets/models/Sponza/glTF/Sponza.gltf", GL_STATIC_DRAW, GL_MIRRORED_REPEAT, app); });*/
        std::jthread m4([&]{ duck.loadGLTF("/home/korei/source/glTF-Sample-Assets/Models/Sponza/glTF/Sponza.gltf", GL_STATIC_DRAW, GL_MIRRORED_REPEAT, app); });
        /*std::jthread m3([&]{ duck.loadGLTF("/home/korei/source/glTF-Sample-Assets/Models/Sponza/glTF/Sponza.gltf", GL_STATIC_DRAW, GL_MIRRORED_REPEAT, app); });*/
        duckTex.loadBMP("test-assets/models/duck/DuckCM.bmp", TEX_TYPE::DIFFUSE, true, GL_MIRRORED_REPEAT, app);
        /*duckTex.loadBMP("test-assets/floor.bmp", diffuse, false, GL_CLAMP_TO_EDGE, app);*/
    }

    /* restore context after assets are loaded */
    app->bindGlContext();
}

f64 incCounter = 0;
f32 fov = 90.0f;
f64 x = 0.0, y = 0.0, z = 0.0;

void
renderScene(Shader* sh, bool depth)
{
    m4 m = m4Iden();

    m = m4Scale(m, 0.01f);
    sh->setM4("uModel", m);
    if (!depth)
    {
        sh->setM3("uNormalMatrix", m3Normal(m));
    }
    /*sponza.drawTex();*/
    duckTex.bind(GL_TEXTURE0);
    duck.drawGLTF();
}

void
drawFrame(App* app)
{
    player.updateDeltaTime();
    player.procMouse();
    player.procKeys(app);

    f32 aspect = (f32)app->wWidth / (f32)app->wHeight;
    constexpr f32 shadowAspect = (f32)SHADOW_WIDTH / (f32)SHADOW_HEIGHT;

    if (!app->bPaused)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        player.updateProj(toRad(fov), aspect, 0.01f, 100.0f);
        player.updateView();
        /* copy both proj and view in one go */
        projView.bufferData(&player, 0, sizeof(m4) * 2);

        // v3 lightPos {x, 4, -1};
        v3 lightPos {(f32)cos(player.currTime) * 6.0f, 3, (f32)sin(player.currTime) * 1.1f};
        constexpr v3 lightColor(Color::snow);
        f32 nearPlane = 0.01f, farPlane = 25.0f;
        m4 shadowProj = m4Pers(toRad(90.0f), shadowAspect, nearPlane, farPlane);
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
        glViewport(0, 0, app->wWidth, app->wHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /* render scene as normal using the denerated depth map */
        omniDirShadowSh.use();
        omniDirShadowSh.setV3("uLightPos", lightPos);
        omniDirShadowSh.setV3("uLightColor", lightColor);
        omniDirShadowSh.setV3("uViewPos", player.pos);
        omniDirShadowSh.setF("uFarPlane", farPlane);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap.tex);
        renderScene(&omniDirShadowSh, false);

        /* draw light source */
        m4 cubeTm = m4Iden();
        cubeTm = m4Translate(cubeTm, lightPos);
        cubeTm = m4Scale(cubeTm, 0.05f);
        colorSh.use();
        colorSh.setM4("uModel", cubeTm);
        colorSh.setV3("uColor", lightColor);
        sphere.drawTex();

        /*texSh.use();*/
        /*texSh.setM4("uModel", m4Scale(m4Iden(), 0.01));*/
        /*duckTex.bind(GL_TEXTURE0);*/
        /*duck.drawGLTF();*/

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
