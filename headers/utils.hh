#pragma once
#include "ultratypes.h"

#include <mutex>
#include <string_view>
#include <vector>
#include <iostream>
#include <format>

enum LogSeverity : int
{
    OK,
    GOOD,
    WARNING,
    BAD,
    FATAL
};

std::vector<char> loadFileToCharArray(std::string_view path, size_t addBytes = 1);
f64 timeNow();
int rngGet(int min, int max);
int rngGet();
f32 rngGet(f32 min, f32 max);
void flipCpyBGRAtoRGBA(u8* dest, u8* src, int width, int height, bool vertFlip);
void flipCpyBGRtoRGB(u8* dest, u8* src, int width, int height, bool vertFlip);
void flipCpyBGRtoRGBA(u8* dest, u8* src, int width, int height, bool vertFlip);
std::string replaceFileSuffixInPath(std::string_view path, std::string_view suffix);

const std::string_view severityStr[FATAL + 1] {
    "",
    "[GOOD]",
    "[WARNING]",
    "[BAD]",
    "[FATAL]"
};

extern std::mutex g_glContextMtx;
extern std::mutex g_logMtx;

#define LEN(A) (sizeof(A) / sizeof(A[0]))
#define ODD(A) (A & 1)
#define EVEN(A) (!ODD(A))

#define FMT std::format
#define COUT std::cout << FMT
#define CERR std::cerr << FMT

#define NPOS static_cast<size_t>(-1UL)

#ifdef LOGS
#    define LOG(severity, ...)                                                                                         \
        do                                                                                                             \
        {                                                                                                              \
            std::lock_guard lock(g_logMtx);                                                                            \
            CERR("{}({}): {} ", __FILE__, __LINE__, severityStr[severity]);                                            \
            CERR(__VA_ARGS__);                                                                                         \
            if (severity == FATAL)                                                                                     \
                abort();                                                                                               \
        } while (0)
#else
#    define LOG(severity, ...) (void)0 /* noop */
#endif

constexpr inline u64
hashFNV(std::string_view str)
{
    u64 hash = 0xCBF29CE484222325;
    for (size_t i = 0; i < str.size(); i++)
        hash = (hash ^ static_cast<u64>(str[i])) * 0x100000001B3;
    return hash;
}
