#pragma once
#include "ultratypes.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <print>
#include <string_view>
#include <vector>

enum LogSeverity : int
{
    OK,
    GOOD,
    WARNING,
    BAD,
    FATAL
};

std::vector<char> fileLoad(std::string_view path, size_t addBytes = 1);
f64 timeNow();
int rngGet(int min, int max);
int rngGet();
f32 rngGet(f32 min, f32 max);

struct Parser
{
    std::string word;
    std::string_view defSeps;
    std::vector<char> file;
    size_t start = 0;
    size_t end = 0;

    Parser(std::string_view path, std::string_view defaultSeparators, size_t addBytes = 1);

    char& operator[](size_t i) { return file[i]; };

    void nextWord(std::string_view separators);
    void nextWord();
    void skipWord(std::string_view separators);
    void skipWord();
    size_t size() const { return file.size(); };
    bool finished();

private:
    bool isSeparator(char c, std::string_view separators);
};

const std::string_view severityStr[FATAL + 1] {
    "",
    "[GOOD]",
    "[WARNING]",
    "[BAD]",
    "[FATAL]"
};

extern GLenum glLastErrorCode;
extern EGLint eglLastErrorCode;

#define LEN(A) (sizeof(A) / sizeof(A[0]))
#define ODD(A) (A & 1)
#define EVEN(A) (!ODD(A))

#ifdef LOGS
#    define LOG(severity, ...)                                                                                         \
        do                                                                                                             \
        {                                                                                                              \
            std::print(stderr, "{}({}): {} ", __FILE__, __LINE__, severityStr[severity]);                              \
            std::print(stderr, __VA_ARGS__);                                                                           \
            if (severity == FATAL)                                                                                     \
                abort();                                                                                               \
        } while (0)
#else
#    define LOG(severity, ...) (void)0;
#endif

#ifdef DEBUG
#    define D(C)                                                                                                       \
        {                                                                                                              \
            /* call function C and check for error, enabled if -DDEBUG and -DLOGS */                                   \
            C;                                                                                                         \
            while ((glLastErrorCode = glGetError()))                                                                   \
            {                                                                                                          \
                switch (glLastErrorCode)                                                                               \
                {                                                                                                      \
                    default:                                                                                           \
                        LOG(WARNING, "unknown error: {:#x}\n", glLastErrorCode);                                       \
                        break;                                                                                         \
                                                                                                                       \
                    case 0x506:                                                                                        \
                        LOG(BAD, "GL_INVALID_FRAMEBUFFER_OPERATION\n");                                                \
                        break;                                                                                         \
                                                                                                                       \
                    case GL_INVALID_ENUM:                                                                              \
                        LOG(BAD, "GL_INVALID_ENUM\n");                                                                 \
                        break;                                                                                         \
                                                                                                                       \
                    case GL_INVALID_VALUE:                                                                             \
                        LOG(BAD, "GL_INVALID_VALUE\n");                                                                \
                        break;                                                                                         \
                                                                                                                       \
                    case GL_INVALID_OPERATION:                                                                         \
                        LOG(BAD, "GL_INVALID_OPERATION\n");                                                            \
                        break;                                                                                         \
                                                                                                                       \
                    case GL_OUT_OF_MEMORY:                                                                             \
                        LOG(BAD, "GL_OUT_OF_MEMORY\n");                                                                \
                        break;                                                                                         \
                }                                                                                                      \
            }                                                                                                          \
        }
#else
#    define D(C) C;
#endif

#ifdef DEBUG
#    define EGLD(C)                                                                                                    \
        {                                                                                                              \
            C;                                                                                                         \
            if ((eglLastErrorCode = eglGetError()) != EGL_SUCCESS)                                                     \
                LOG(WARNING, "eglLastErrorCode: {:#x}\n", eglLastErrorCode);                                           \
        }
#else
#    define EGLD(C) C
#endif

#define COLOR(hex)                                                                                                     \
    {                                                                                                                  \
        ((hex >> 24) & 0xFF) / 255.0f, ((hex >> 16) & 0xFF) / 255.0f, ((hex >> 8) & 0xFF) / 255.0f,                    \
            (hex & 0xFF) / 255.0f                                                                                      \
    }

inline constexpr size_t
hashFNV(std::string_view str)
{
	size_t hash = 0xCBF29CE484222325;
	for (size_t i = 0; i < str.size(); i++)
        hash = (hash ^ str[i]) * 0x100000001B3;
	return hash;
}
