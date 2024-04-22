#include "headers/utils.hh"

#include <fstream>
#include <chrono>
#include <random>
#include <mutex>
#include <climits>

std::mutex glContextMtx;
std::mutex logMtx;

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

#ifdef __linux__
__attribute__((no_sanitize("undefined"))) /* complains about unaligned pointers */
#endif
void
flipCpyBGRAtoRGBA(u8* dest, u8* src, int width, int height, bool vertFlip)
{
    int f = vertFlip ? -(height - 1) : 0;
    int inc = vertFlip ? 2 : 0;

    u32* d = (u32*)dest;
    u32* s = (u32*)src;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            /* take 4 bytes at once then swap red and blue bits */
            u32 t = s[y*width + x];
            u32 R =   t & 0x00'ff'00'00;
            u32 B =   t & 0x00'00'00'ff;
            u32 tt = (t & 0xff'00'ff'00) | (R >> (4*4)) | (B << (4*4));
            d[(y-f)*width + x] = tt;
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
        return (y-f)*width + x + z;
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

    u8* d = (u8*)dest;
    u8* s = (u8*)src;

    constexpr int nComponents = 3;
    width = width * nComponents;

    width = width*nComponents;

    auto at = [=](int x, int y, int z) -> int
    {
        return (y-f)*width + x + z;
    };

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x += nComponents)
        {
            d[at(x, y-f, 0)] = s[at(x, y, 2)];
            d[at(x, y-f, 1)] = s[at(x, y, 1)];
            d[at(x, y-f, 2)] = s[at(x, y, 0)];
            d[at(x, y-f, 3)] = 0xff;
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
replaceFileSuffixInPath(std::string_view path, std::string* suffix)
{
#ifdef _WIN32
    suffix->back() = '\0';
#endif

    auto lastSlash = path.find_last_of("/");
    std::string pathToMtl {path.begin(), path.begin() + lastSlash};

    return {pathToMtl + "/" + (*suffix)};
}
