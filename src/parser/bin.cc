#include "bin.hh"

namespace parser
{

Binary::Binary(std::string_view path)
{
    loadFile(path);
}

void 
Binary::loadFile(std::string_view path)
{
    start = end = 0;
    file = loadFileToString(path);
}

void 
Binary::skipBytes(size_t n)
{
    start = end = end + n;
}

std::string
Binary::readString(size_t size)
{
    std::string ret(file.begin() + start, file.begin() + start + size);
    start = end = end + size;
    return ret;
}

u8
Binary::read8()
{
    auto ret = readTypeBytes<u8>(file, start);
    start = end = end + 1;
    return ret;
}

u16
Binary::read16()
{
    auto ret = readTypeBytes<u16>(file, start);
    start = end = end + 2;
    return ret;
}

u32
Binary::read32()
{
    auto ret = readTypeBytes<u32>(file, start);
    start = end = end + 4;
    return ret;
}

u64
Binary::read64()
{
    auto ret = readTypeBytes<u64>(file, start);
    start = end = end + 8;
    return ret;
}

void
Binary::setPos(size_t p)
{
    start = end = p;
}

bool
Binary::finished()
{
    return start >= size();
}

} /* namespace parser */
