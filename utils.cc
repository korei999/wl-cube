#include "headers/utils.hh"

#include <fstream>
#include <chrono>
#include <random>

static std::mt19937 rngCreate();

GLenum glLastErrorCode = 0;
EGLint eglLastErrorCode = EGL_SUCCESS;

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

void
flipCpyBGRAtoRGBA(u8* dest, u8* src, int width, int height, bool flip)
{
    int f = flip ? -(height - 1) : 0;
    int inc = flip ? 2 : 0;

    /* C99 vla, clang doesn't allow to declare this non-standard type, but using auto just works */
    auto d = (u32 (*)[width])dest;
    auto s = (u32 (*)[width])src;

    for (int r = 0; r < height; r++)
    {
        for (int c = 0; c < width; c++)
        {
            u32 t = s[r][c];
            /* take 4 bytes and swap red and blue channels */
            u32 R =   t & 0x00FF0000;
            u32 B =   t & 0x000000FF;
            u32 tt = (t & 0xFF00FF00) | (R >> (4 * 4)) | (B << (4 * 4));
            d[r -f][c] = tt;
        }
        f += inc;
    }
};

void
flipCpyBGRtoRGB(u8* dest, u8* src, int width, int height, bool flip)
{
    int f = flip ? -(height - 1) : 0;
    int inc = flip ? 2 : 0;

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

std::vector<char>
fileLoad(std::string_view path, size_t addBytes)
{
    std::ifstream file(path, std::ios::in | std::ios::ate | std::ios::binary);
    if (!file.is_open())
        LOG(FATAL, "failed to open file: {}\n", path);

    size_t fileSize = file.tellg();
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
    typedef std::chrono::duration<f64> fsec;

    fsec fs = Time::now().time_since_epoch();
    return fs.count();
}

Parser::Parser(std::string_view path, std::string_view defaultSeparators, size_t addBytes)
    : defSeps(defaultSeparators)
{
    file = fileLoad(path, addBytes);
}

void
Parser::nextWord(std::string_view separators)
{
    while (file[end] && !isSeparator(file[end], separators))
        end++;

    word = std::string(file.begin() + start, file.begin() + end);
    start = end = end + 1;
}

void
Parser::nextWord()
{
    nextWord(defSeps);
}

void
Parser::skipWord(std::string_view separators)
{
    while (file[end] && !isSeparator(file[end], separators))
        end++;

    start = end = end + 1;
}

void
Parser::skipWord()
{
    skipWord(defSeps);
}

void 
Parser::skipBytes(size_t n)
{
    start = end = end + n;
}

std::string
Parser::readString(size_t size)
{
    std::string ret(file.begin() + start, file.begin() + start + size);
    start = end = end + size;
    return ret;
}

u8
Parser::read8()
{
    auto ret = readTypeBytes<u8>(file, start);
    start = end = end + 1;
    return ret;
}

u16
Parser::read16()
{
    auto ret = readTypeBytes<u16>(file, start);
    start = end = end + 2;
    return ret;
}

u32
Parser::read32()
{
    auto ret = readTypeBytes<u32>(file, start);
    start = end = end + 4;
    return ret;
}

u64
Parser::read64()
{
    auto ret = readTypeBytes<u64>(file, start);
    start = end = end + 8;
    return ret;
}

void
Parser::setPos(size_t p)
{
    start = end = p;
}

bool
Parser::finished()
{
    return start >= size();
}

bool
Parser::isSeparator(char c, std::string_view separotors)
{
    if (!file[c])
        return false;

    for (char i : separotors)
        if (i == c)
            return true;

    return false;
}
