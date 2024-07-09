#include <fstream>
#include <chrono>
#include <random>
#include <mutex>
#include <climits>
#include <immintrin.h>

#include "utils.hh"

std::mutex g_glContextMtx;
std::mutex g_logMtx;

static std::mt19937 rngCreate();

static std::mt19937 mt {rngCreate()};

static std::mt19937
rngCreate()
{
    std::random_device rd {};
    std::seed_seq ss {
        (std::seed_seq::result_type)(timeNow()), rd(), rd(), rd(), rd(), rd(), rd(), rd()
    };

    return std::mt19937(ss);
}

int
rngGet(int min, int max)
{
    return std::uniform_int_distribution {min, max}(mt);
}

int
rngGet()
{
    return std::uniform_int_distribution {INT_MIN, INT_MAX}(mt);
}

f32
rngGet(f32 min, f32 max)
{
    return std::uniform_real_distribution {min, max}(mt);
}

/* complains about unaligned address */
void
flipCpyBGRAtoRGBA(u8* dest, u8* src, int width, int height, bool vertFlip)
{
    int f = vertFlip ? -(height - 1) : 0;
    int inc = vertFlip ? 2 : 0;

    u32* d = reinterpret_cast<u32*>(dest);
    u32* s = reinterpret_cast<u32*>(src);

    auto swapRedBlueBits = [](u32 col) -> u32
    {
        u32 r = col & 0x00'ff'00'00;
        u32 b = col & 0x00'00'00'ff;
        return (col & 0xff'00'ff'00) | (r >> (4*4)) | (b << (4*4));
    };

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x += 4)
        {
            u32 colorsPack[4];
            for (size_t i = 0; i < std::size(colorsPack); i++)
                colorsPack[i] = swapRedBlueBits(s[y*width + x + i]);

            auto _dest = reinterpret_cast<__m128i_u*>(&d[(y-f)*width + x]);
            _mm_storeu_si128(_dest, *reinterpret_cast<__m128i*>(colorsPack));
        }

        f += inc;
    }
};

void
flipCpyBGRtoRGB(u8* dest, u8* src, int width, int height, bool vertFlip)
{
    int f = vertFlip ? -(height - 1) : 0;
    int inc = vertFlip ? 2 : 0;

    constexpr int nComponents = 3;
    width = width * nComponents;

    auto at = [=](int x, int y, int z) -> int
    {
        return y*width + x + z;
    };

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x += nComponents)
        {
            dest[at(x, y-f, 0)] = src[at(x, y, 2)];
            dest[at(x, y-f, 1)] = src[at(x, y, 1)];
            dest[at(x, y-f, 2)] = src[at(x, y, 0)];
        }
        f += inc;
    }
};

void
flipCpyBGRtoRGBA(u8* dest, u8* src, int width, int height, bool vertFlip)
{
    int f = vertFlip ? -(height - 1) : 0;
    int inc = vertFlip ? 2 : 0;

    constexpr int rgbComp = 3;
    constexpr int rgbaComp = 4;

    int rgbWidth = width * rgbComp;
    int rgbaWidth = width * rgbaComp;

    auto at = [](int width, int x, int y, int z) -> int
    {
        return y*width + x + z;
    };

    for (int y = 0; y < height; y++)
    {
        for (int xSrc = 0, xDest = 0; xSrc < rgbWidth; xSrc += rgbComp, xDest += rgbaComp)
        {
            dest[at(rgbaWidth, xDest, y-f, 0)] = src[at(rgbWidth, xSrc, y, 2)];
            dest[at(rgbaWidth, xDest, y-f, 1)] = src[at(rgbWidth, xSrc, y, 1)];
            dest[at(rgbaWidth, xDest, y-f, 2)] = src[at(rgbWidth, xSrc, y, 0)];
            dest[at(rgbaWidth, xDest, y-f, 3)] = 0xff;
        }
        f += inc;
    }
};

static std::mutex fileMtx;

std::vector<char>
loadFileToCharArray(std::string_view path, size_t addBytes)
{
    std::lock_guard lock(fileMtx);

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

f64
timeNow()
{
    typedef std::chrono::high_resolution_clock Time;
    typedef std::chrono::duration<f64> Fsec;

    Fsec fs = Time::now().time_since_epoch();
    return fs.count();
}

std::string
replacePathSuffix(std::string_view path, std::string_view suffix)
{
#ifdef _WIN32
    suffix->back() = '\0';
#endif

    auto lastSlash = path.find_last_of("/");
    std::string pathToMtl {path.begin(), path.begin() + lastSlash};

    return {pathToMtl + "/" + std::string(suffix)};
}
