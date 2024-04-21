#include "windows.hh"
#include "../../headers/utils.hh"
#include "../../headers/gl.hh"
#include "wglext.h" /* https://www.khronos.org/registry/OpenGL/api/GL/wglext.h */

#include <cassert>

// #define GL_FUNCTIONS(X) \
    // X(PFNGLCREATEBUFFERSPROC,            glCreateBuffers            ) \
    // X(PFNGLNAMEDBUFFERSTORAGEPROC,       glNamedBufferStorage       ) \
    // X(PFNGLBINDVERTEXARRAYPROC,          glBindVertexArray          ) \
    // X(PFNGLCREATEVERTEXARRAYSPROC,       glCreateVertexArrays       ) \
    // X(PFNGLVERTEXARRAYATTRIBBINDINGPROC, glVertexArrayAttribBinding ) \
    // X(PFNGLVERTEXARRAYVERTEXBUFFERPROC,  glVertexArrayVertexBuffer  ) \
    // X(PFNGLVERTEXARRAYATTRIBFORMATPROC,  glVertexArrayAttribFormat  ) \
    // X(PFNGLENABLEVERTEXARRAYATTRIBPROC,  glEnableVertexArrayAttrib  ) \
    // X(PFNGLCREATESHADERPROGRAMVPROC,     glCreateShaderProgramv     ) \
    // X(PFNGLGETPROGRAMIVPROC,             glGetProgramiv             ) \
    // X(PFNGLGETPROGRAMINFOLOGPROC,        glGetProgramInfoLog        ) \
    // X(PFNGLGENPROGRAMPIPELINESPROC,      glGenProgramPipelines      ) \
    // X(PFNGLUSEPROGRAMSTAGESPROC,         glUseProgramStages         ) \
    // X(PFNGLBINDPROGRAMPIPELINEPROC,      glBindProgramPipeline      ) \
    // X(PFNGLPROGRAMUNIFORMMATRIX2FVPROC,  glProgramUniformMatrix2fv  ) \
    // X(PFNGLBINDTEXTUREUNITPROC,          glBindTextureUnit          ) \
    // X(PFNGLCREATETEXTURESPROC,           glCreateTextures           ) \
    // X(PFNGLTEXTUREPARAMETERIPROC,        glTextureParameteri        ) \
    // X(PFNGLTEXTURESTORAGE2DPROC,         glTextureStorage2D         ) \
    // X(PFNGLTEXTURESUBIMAGE2DPROC,        glTextureSubImage2D        ) \
    // X(PFNGLDEBUGMESSAGECALLBACKPROC,     glDebugMessageCallback     ) \
    // X(PFNGLGENFRAMEBUFFERSPROC,          glGenFramebuffers          ) \
    // X(PFNGLBINDFRAMEBUFFERPROC,          glBindFramebuffer          ) \
    // X(PFNGLACTIVETEXTUREPROC,            glActiveTexture            ) \
    // X(PFNGLDELETEVERTEXARRAYSPROC,       glDeleteVertexArrays       ) \
    // X(PFNGLDRAWBUFFERSPROC,              glDrawBuffers              ) \
    // X(PFNGLGENVERTEXARRAYSPROC,          glGenVertexArrays          ) \
    // X(PFNGLGENBUFFERSPROC,               glGenBuffers               ) \
    // X(PFNGLBINDBUFFERPROC,               glBindBuffer               ) \
    // X(PFNGLBUFFERDATAPROC,               glBufferData               ) \
    // X(PFNGLENABLEVERTEXATTRIBARRAYPROC,  glEnableVertexAttribArray  ) \
    // X(PFNGLVERTEXATTRIBPOINTERPROC,      glVertexAttribPointer      ) \
    // X(PFNGLDRAWELEMENTSINSTANCEDPROC,    glDrawElementsInstanced    ) \
    // X(PFNGLGENBUFFERSPROC,               glDeleteBuffers            ) \
    // X(PFNGLGETUNIFORMBLOCKINDEXPROC,     glGetUniformBlockIndex     ) \
    // X(PFNGLUNIFORMBLOCKBINDINGPROC,      glUniformBlockBinding      ) \
    // X(PFNGLCREATEPROGRAMPROC,            glCreateProgram            ) \
    // X(PFNGLATTACHSHADERPROC,             glAttachShader             ) \
    // X(PFNGLLINKPROGRAMPROC,              glLinkProgram              ) \
    // X(PFNGLDELETEPROGRAMPROC,            glDeleteProgram            ) \
    // X(PFNGLVALIDATEPROGRAMPROC,          glValidateProgram          ) \
    // X(PFNGLDELETESHADERPROC,             glDeleteShader             ) \
    // X(PFNGLCREATESHADERPROC,             glCreateShader             ) \
    // X(PFNGLSHADERSOURCEPROC,             glShaderSource             ) \
    // X(PFNGLCOMPILESHADERPROC,            glCompileShader            ) \
    // X(PFNGLGETSHADERIVPROC,              glGetShaderiv              ) \
    // X(PFNGLGETSHADERINFOLOGPROC,         glGetShaderInfoLog         ) \
    // X(PFNGLUSEPROGRAMPROC,               glUseProgram               ) \
    // X(PFNGLGETACTIVEUNIFORMPROC,         glGetActiveUniform         ) \
    // X(PFNGLBINDBUFFERBASEPROC,           glBindBufferBase           ) \
    // X(PFNGLBUFFERSUBDATAPROC,            glBufferSubData            ) \
    // X(PFNGLGETUNIFORMLOCATIONPROC,       glGetUniformLocation       ) \
    // X(PFNGLUNIFORMMATRIX4FVPROC,         glUniformMatrix4fv         ) \
    // X(PFNGLUNIFORMMATRIX3FVPROC,         glUniformMatrix3fv         ) \
    // X(PFNGLUNIFORM3FVPROC,               glUniform3fv               ) \
    // X(PFNGLUNIFORM1IPROC,                glUniform1i                ) \
    // X(PFNGLUNIFORM1FPROC,                glUniform1f                ) \
    // X(PFNGLGENERATEMIPMAPPROC,           glGenerateMipmap           ) \
    // X(PFNGLFRAMEBUFFERTEXTURE2DPROC,     glFramebufferTexture2D     ) \
    // X(PFNGLCHECKFRAMEBUFFERSTATUSPROC,   glCheckFramebufferStatus   ) \
    // X(PFNGLFRAMEBUFFERTEXTUREPROC,       glFramebufferTexture       )

// #define X(type, name) static type name;
// GL_FUNCTIONS(X)
// #undef X

#define STR2(x) #x
#define STR(x) STR2(x)

static void
FatalError(const char* message)
{
    MessageBoxA(nullptr, message, "Error", MB_ICONEXCLAMATION);
    ExitProcess(0);
}

static LRESULT CALLBACK
WindowProc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(wnd, msg, wparam, lparam);
}

// compares src string with dstlen characters from dst, returns 1 if they are equal, 0 if not
static int
StringsAreEqual(const char* src, const char* dst, size_t dstlen)
{
    while (*src && dstlen-- && *dst)
    {
        if (*src++ != *dst++)
        {
            return 0;
        }
    }

    return (dstlen && *src == *dst) || (!dstlen && *src == 0);
}

static PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = nullptr;
static PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;
static PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = nullptr;

static void
GetWglFunctions(void)
{
    /* to get WGL functions we need valid GL context, so create dummy window for dummy GL contetx */
    HWND dummy = CreateWindowExW(
        0, L"STATIC", L"DummyWindow", WS_OVERLAPPED,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        nullptr, nullptr, nullptr, nullptr);
    if (!dummy)
        LOG(FATAL, "CreateWindowExW failed\n");

    HDC dc = GetDC(dummy);
    if (!dc)
        LOG(FATAL, "GetDC failed\n");

    PIXELFORMATDESCRIPTOR desc {
        .nSize = sizeof(desc),
        .nVersion = 1,
        .dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        .iPixelType = PFD_TYPE_RGBA,
        .cColorBits = 24,
    };

    int format = ChoosePixelFormat(dc, &desc);
    if (!format)
        LOG(FATAL, "Cannot choose OpenGL pixel format for dummy window!");

    int ok = DescribePixelFormat(dc, format, sizeof(desc), &desc);
    if (!ok)
        LOG(FATAL, "DescribePixelFormat failed\n");

    // reason to create dummy window is that SetPixelFormat can be called only once for the window
    if (!SetPixelFormat(dc, format, &desc))
        LOG(FATAL, "Cannot set OpenGL pixel format for dummy window!");

    HGLRC rc = wglCreateContext(dc);
    if (!rc)
        LOG(FATAL, "wglCreateContext failed\n");

    ok = wglMakeCurrent(dc, rc);
    if (!ok)
        LOG(FATAL, "wglMakeCurrent failed\n");

    // https://www.khronos.org/registry/OpenGL/extensions/ARB/WGL_ARB_extensions_string.txt
    auto wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)wglGetProcAddress("wglGetExtensionsStringARB");
    if (!wglGetExtensionsStringARB)
    {
        FatalError("OpenGL does not support WGL_ARB_extensions_string extension!");
    }

    const char* ext = wglGetExtensionsStringARB(dc);
    if (!ext)
        LOG(FATAL, "wglGetExtensionsStringARB failed\n");

    const char* start = ext;
    for (;;)
    {
        while (*ext != 0 && *ext != ' ')
            ext++;

        size_t length = ext - start;
        if (StringsAreEqual("WGL_ARB_pixel_format", start, length))
        {
            // https://www.khronos.org/registry/OpenGL/extensions/ARB/WGL_ARB_pixel_format.txt
            wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
        }
        else if (StringsAreEqual("WGL_ARB_create_context", start, length))
        {
            // https://www.khronos.org/registry/OpenGL/extensions/ARB/WGL_ARB_create_context.txt
            wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
        }
        else if (StringsAreEqual("WGL_EXT_swap_control", start, length))
        {
            // https://www.khronos.org/registry/OpenGL/extensions/EXT/WGL_EXT_swap_control.txt
            wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
        }

        if (*ext == 0)
            break;

        ext++;
        start = ext;
    }

    if (!wglChoosePixelFormatARB || !wglCreateContextAttribsARB || !wglSwapIntervalEXT)
    {
        FatalError("OpenGL does not support required WGL extensions for modern context!");
    }

    wglMakeCurrent(nullptr, nullptr);
    wglDeleteContext(rc);
    ReleaseDC(dummy, dc);
    DestroyWindow(dummy);
}

Win32window::Win32window(std::string_view name, HINSTANCE _instance)
{
    this->name = name;
    this->instance = _instance;
    init();
}

Win32window::~Win32window()
{
}

void
Win32window::init()
{
    GetWglFunctions();

    wc = {
        .cbSize = sizeof(wc),
        .lpfnWndProc = WindowProc,
        .hInstance = instance,
        .hIcon = LoadIcon(nullptr, IDI_APPLICATION),
        .hCursor = LoadCursor(nullptr, IDC_ARROW),
        .lpszClassName = L"opengl_window_class",
    };

    ATOM atom = RegisterClassExW(&wc);
    if (!atom)
        LOG(FATAL, "RegisterClassExW failed\n");

    this->wWidth = 1240;
    this->wHeight = 960;
    DWORD exstyle = WS_EX_APPWINDOW;
    DWORD style = WS_OVERLAPPEDWINDOW;

    /* uncomment in case you want fixed size window */
    /* style &= ~WS_THICKFRAME & ~WS_MAXIMIZEBOX; */
    /* RECT rect = { 0, 0, 1280, 720 }; */
    /* AdjustWindowRectEx(&rect, style, FALSE, exstyle); */
    /* width = rect.right - rect.left; */
    /* height = rect.bottom - rect.top; */

    window = CreateWindowExW(exstyle,
                             wc.lpszClassName,
                             L"OpenGL Window",
                             style,
                             CW_USEDEFAULT,
                             CW_USEDEFAULT,
                             this->wWidth,
                             this->wHeight,
                             nullptr,
                             nullptr,
                             wc.hInstance,
                             nullptr);
    if (!window)
        LOG(FATAL, "CreateWindowExW failed\n");

    deviceContext = GetDC(window);
    if (!deviceContext)
        LOG(FATAL, "GetDC failed\n");

    int attrib[] {
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
        WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB,     24,
        WGL_DEPTH_BITS_ARB,     24,
        WGL_STENCIL_BITS_ARB,   8,

        // uncomment for sRGB framebuffer, from WGL_ARB_framebuffer_sRGB extension
        // https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_framebuffer_sRGB.txt
        //WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, GL_TRUE,

        // uncomment for multisampeld framebuffer, from WGL_ARB_multisample extension
        // https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_multisample.txt
        //WGL_SAMPLE_BUFFERS_ARB, 1,
        //WGL_SAMPLES_ARB,        4, // 4x MSAA

        0,
    };

    int format;
    UINT formats;
    if (!wglChoosePixelFormatARB(deviceContext, attrib, nullptr, 1, &format, &formats) || formats == 0)
        LOG(FATAL, "OpenGL does not support required pixel format!");

    PIXELFORMATDESCRIPTOR desc { .nSize = sizeof(desc) };
    int ok = DescribePixelFormat(deviceContext, format, sizeof(desc), &desc);
    if (!ok)
        LOG(FATAL, "DescribePixelFormat failed\n");

    if (!SetPixelFormat(deviceContext, format, &desc))
        LOG(FATAL, "Cannot set OpenGL selected pixel format!");

    int attribContext[] {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
        WGL_CONTEXT_MINOR_VERSION_ARB, 5,
        WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
#ifdef DEBUG
        // ask for debug context for non "Release" builds
        // this is so we can enable debug callback
        WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
#endif
        0,
    };

    glContext = wglCreateContextAttribsARB(deviceContext, nullptr, attribContext);
    if (!glContext)
        LOG(FATAL, "Cannot create modern OpenGL context! OpenGL version 4.5 not supported?");

    bool okContext = wglMakeCurrent(deviceContext, glContext);
    if (!okContext)
        LOG(FATAL, "wglMakeCurrent failed\n");

    // load OpenGL functions
// #define X(type, name) name = (type)wglGetProcAddress(#name); assert(name);
    // GL_FUNCTIONS(X)
// #undef X
    auto gladOk = gladLoadGL();
    if (!gladOk)
        LOG(FATAL, "gladLoadGL failed\n");

    unbindGlContext();
}

void
Win32window::disableRelativeMode()
{
}

void
Win32window::enableRelativeMode()
{
}

void
Win32window::togglePointerRelativeMode()
{
}

void
Win32window::toggleFullscreen()
{
}

void 
Win32window::setCursorImage(std::string_view cursorType)
{
}

void 
Win32window::setFullscreen() 
{
}

void
Win32window::unsetFullscreen()
{
}

void 
Win32window::bindGlContext()
{
    wglMakeCurrent(deviceContext, glContext);
}

void 
Win32window::unbindGlContext()
{
    wglMakeCurrent(nullptr, nullptr);
}

void
Win32window::setSwapInterval(int interval)
{
    swapInterval = interval;
    wglSwapIntervalEXT(interval);
}

void 
Win32window::toggleVSync()
{
    swapInterval = !swapInterval; 
    wglSwapIntervalEXT(swapInterval);
}

void
Win32window::swapBuffers()
{
    if (!SwapBuffers(deviceContext))
        LOG(WARNING, "SwapBuffers(dc): failed\n");
}

void 
Win32window::procEvents()
{
    MSG msg;
    if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
            throw 0;

        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

void
Win32window::showWindow()
{
    ShowWindow(window, SW_SHOWDEFAULT);
}
