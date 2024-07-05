#include "parser.hh"

ObjParser::ObjParser(std::string_view defaultSeparators)
    : defSeps(defaultSeparators) {}

ObjParser::ObjParser(std::string_view path, std::string_view defaultSeparators, size_t addZeroBytes)
    : defSeps(defaultSeparators)
{
    loadFile(path, addZeroBytes);
}

void 
ObjParser::loadFile(std::string_view path, size_t addZeroBytes)
{
    start = end = 0;
    file = loadFileToCharArray(path, addZeroBytes);
}

void
ObjParser::nextWord(std::string_view separators)
{
    while (file[end] && !isSeparator(file[end], separators))
        end++;

    word = {file.begin() + start, file.begin() + end};
    start = end = end + 1;
}

void
ObjParser::nextWord()
{
    nextWord(defSeps);
}

void
ObjParser::skipWord(std::string_view separators)
{
    while (file[end] && !isSeparator(file[end], separators))
        end++;

    start = end = end + 1;
}

void
ObjParser::skipWord()
{
    skipWord(defSeps);
}

void 
ObjParser::skipBytes(size_t n)
{
    start = end = end + n;
}

std::string
ObjParser::readString(size_t size)
{
    std::string ret(file.begin() + start, file.begin() + start + size);
    start = end = end + size;
    return ret;
}

u8
ObjParser::read8()
{
    auto ret = readTypeBytes<u8>(file, start);
    start = end = end + 1;
    return ret;
}

u16
ObjParser::read16()
{
    auto ret = readTypeBytes<u16>(file, start);
    start = end = end + 2;
    return ret;
}

u32
ObjParser::read32()
{
    auto ret = readTypeBytes<u32>(file, start);
    start = end = end + 4;
    return ret;
}

u64
ObjParser::read64()
{
    auto ret = readTypeBytes<u64>(file, start);
    start = end = end + 8;
    return ret;
}

void
ObjParser::setPos(size_t p)
{
    start = end = p;
}

bool
ObjParser::finished()
{
    return start >= size();
}

bool
ObjParser::isSeparator(char c, std::string_view separotors)
{
    if (!file[c])
        return false;

    for (char i : separotors)
        if (i == c)
            return true;

    return false;
}

void
ObjParser::skipWhiteSpace()
{
    while (file[end] && (file[end] == ' ' || file[end] == '\n'))
        end++;

    start = end;
}
