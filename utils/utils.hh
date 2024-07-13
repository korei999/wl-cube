#pragma once

#include <chrono>
#include <fstream>
#include <string_view>
#include <vector>
#include <iostream>
#include <format>
#include <cmath>

#include "ultratypes.h"

enum LogSeverity : int
{
    OK,
    GOOD,
    WARNING,
    BAD,
    FATAL
};

constexpr std::string_view severityStr[FATAL + 1] {
    "",
    "[GOOD]",
    "[WARNING]",
    "[BAD]",
    "[FATAL]"
};

#define LEN(A) (sizeof(A) / sizeof(A[0]))
#define ODD(A) (A & 1)
#define EVEN(A) (!ODD(A))

#define FMT std::format
#define COUT std::cout << FMT
#define CERR std::cerr << FMT

#define NPOS static_cast<size_t>(-1UL)

#ifdef LOGS
    #define LOG(severity, ...)                                                                                         \
        do                                                                                                             \
        {                                                                                                              \
            CERR("{}({}): {}", __FILE__, __LINE__, severityStr[severity]);                                             \
            CERR(__VA_ARGS__);                                                                                         \
            if (severity == FATAL)                                                                                     \
                abort();                                                                                               \
        } while (0)
#else
    #define LOG(severity, ...) (void)0 /* noop */
#endif

constexpr inline u64
hashFNV(std::string_view str)
{
    u64 hash = 0xCBF29CE484222325;
    for (size_t i = 0; i < str.size(); i++)
        hash = (hash ^ static_cast<u64>(str[i])) * 0x100000001B3;
    return hash;
}

static inline std::vector<char>
loadFileToCharArray(std::string_view path, size_t addBytes = 1)
{
    std::ifstream file(path.data(), std::ios::in | std::ios::ate | std::ios::binary);
    if (!file.is_open())
        LOG(FATAL, "failed to open file: '{}'\n", path);

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize + addBytes, '\0');

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

static inline std::string
loadFileToString(std::string_view path)
{
    std::ifstream file(path.data(), std::ios::in | std::ios::ate | std::ios::binary);
    if (!file.is_open())
        LOG(FATAL, "failed to open file: '{}'\n", path);

    size_t fileSize = (size_t)file.tellg();
    std::string buffer = std::string(fileSize + 1, '\0');

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

static inline f64
timeNowS()
{
    long t =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()
        ).count();

    return static_cast<f64>(t) / 1000.0;
}

static inline std::string
replacePathSuffix(std::string_view path, std::string_view suffix)
{
    auto lastSlash = path.find_last_of("/");
    std::string_view pathToMtl = std::string_view(path).substr(0, lastSlash);

    return FMT("{}/{}", pathToMtl, suffix);
}
