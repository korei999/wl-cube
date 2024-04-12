#pragma once
#include "ultratypes.h"

#include <GLES3/gl32.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <print>
#include <string_view>
#include <vector>
#include <mutex>

enum LogSeverity : int
{
    OK,
    GOOD,
    WARNING,
    BAD,
    FATAL
};

extern std::mutex glContextMtx;

std::vector<char> loadFileToCharArray(std::string_view path, size_t addBytes = 1);
f64 timeNow();
int rngGet(int min, int max);
int rngGet();
f32 rngGet(f32 min, f32 max);
void flipCpyBGRAtoRGBA(u8* dest, u8* src, int width, int height, bool vertFlip);
void flipCpyBGRtoRGB(u8* dest, u8* src, int width, int height, bool vertFlip);
void flipCpyBGRtoRGBA(u8* dest, u8* src, int width, int height, bool vertFlip);

template <typename type>
__attribute__((no_sanitize("undefined"))) /* can complain about unaligned pointers */
type
readTypeBytes(const std::vector<char>& vec, size_t i)
{
    return *(type*)&vec[i];
}

struct Parser
{
    std::string word;
    std::string_view defSeps;
    std::vector<char> file;
    size_t start;
    size_t end;

    Parser(std::string_view defaultSeparators);
    Parser(std::string_view path, std::string_view defaultSeparators, size_t addBytes = 1);

    char& operator[](size_t i) { return file[i]; };

    void loadFile(std::string_view path, size_t addBytes = 1);
    void nextWord(std::string_view separators);
    void nextWord();
    void skipWord(std::string_view separators);
    void skipWord();
    void skipBytes(size_t n);
    std::string readString(size_t size);
    u8 read8();
    u16 read16();
    u32 read32();
    u64 read64();
    void setPos(size_t p);
    size_t size() const { return file.size(); };
    bool finished();
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
#    define LOG(severity, ...)
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
                LOG(FATAL, "eglLastErrorCode: {:#x}\n", eglLastErrorCode);                                             \
        }
#else
#    define EGLD(C) C
#endif

#define COLOR4(hex)                                                                                                    \
    {                                                                                                                  \
        ((hex >> 24) & 0xFF) / 255.0f,                                                                                 \
        ((hex >> 16) & 0xFF) / 255.0f,                                                                                 \
        ((hex >> 8)  & 0xFF) / 255.0f,                                                                                 \
        ((hex >> 0)  & 0xFF) / 255.0f                                                                                  \
    }

#define COLOR3(hex)                                                                                                    \
    {                                                                                                                  \
        ((hex >> 16) & 0xFF) / 255.0f,                                                                                 \
        ((hex >> 8)  & 0xFF) / 255.0f,                                                                                 \
        ((hex >> 0)  & 0xFF) / 255.0f                                                                                  \
    }

constexpr inline size_t
hashFNV(std::string_view str)
{
	size_t hash = 0xCBF29CE484222325;
	for (size_t i = 0; i < str.size(); i++)
        hash = (hash ^ str[i]) * 0x100000001B3;
	return hash;
}

std::string replaceFileSuffixInPath(std::string_view str, std::string_view suffix);
