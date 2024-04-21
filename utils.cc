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

__attribute__((no_sanitize("undefined"))) /* complains about unaligned pointers */
void
flipCpyBGRAtoRGBA(u8* dest, u8* src, int width, int height, bool vertFlip)
{
    int f = vertFlip ? -(height - 1) : 0;
    int inc = vertFlip ? 2 : 0;

    /* C99 vla */
    auto d = (u32 (*)[width])dest;
    auto s = (u32 (*)[width])src;

    for (int r = 0; r < height; r++)
    {
        for (int c = 0; c < width; c++)
        {
            /* take 4 bytes at once then swap red and blue bits */
            u32 t = s[r][c];
            u32 R =   t & 0x00'ff'00'00;
            u32 B =   t & 0x00'00'00'ff;
            u32 tt = (t & 0xff'00'ff'00) | (R >> (4*4)) | (B << (4*4));
            d[r - f][c] = tt;
        }
        f += inc;
    }
};

void
flipCpyBGRtoRGB(u8* dest, u8* src, int width, int height, bool vertFlip)
{
    int f = vertFlip ? -(height - 1) : 0;
    int inc = vertFlip ? 2 : 0;

    auto d = (u8 (*)[width][3])dest;
    auto s = (u8 (*)[width][3])src;

    for (int r = 0; r < height; r++)
    {
        for (int c = 0; c < width; c++)
        {
            d[r - f][c][0] = s[r][c][2];
            d[r - f][c][1] = s[r][c][1];
            d[r - f][c][2] = s[r][c][0];
        }
        f += inc;
    }
};

void
flipCpyBGRtoRGBA(u8* dest, u8* src, int width, int height, bool vertFlip)
{
    int f = vertFlip ? -(height - 1) : 0;
    int inc = vertFlip ? 2 : 0;

    auto d = (u8 (*)[width][4])dest;
    auto s = (u8 (*)[width][3])src;

    for (int r = 0; r < height; r++)
    {
        for (int c = 0; c < width; c++)
        {
            d[r - f][c][0] = s[r][c][2];
            d[r - f][c][1] = s[r][c][1];
            d[r - f][c][2] = s[r][c][0];
            d[r - f][c][3] = 0xff;
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
