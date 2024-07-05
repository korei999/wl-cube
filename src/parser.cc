#include "parser.hh"

GenParser::GenParser(std::string_view defaultSeparators)
    : defSeps(defaultSeparators) {}

GenParser::GenParser(std::string_view path, std::string_view defaultSeparators, size_t addZeroBytes)
    : defSeps(defaultSeparators)
{
    loadFile(path, addZeroBytes);
}

void 
GenParser::loadFile(std::string_view path, size_t addZeroBytes)
{
    start = end = 0;
    file = loadFileToCharArray(path, addZeroBytes);
}

void
GenParser::nextWord(std::string_view separators)
{
    while (file[end] && !isSeparator(file[end], separators))
        end++;

    word = {file.begin() + start, file.begin() + end};
    start = end = end + 1;
}

void
GenParser::nextWord()
{
    nextWord(defSeps);
}

void
GenParser::skipWord(std::string_view separators)
{
    while (file[end] && !isSeparator(file[end], separators))
        end++;

    start = end = end + 1;
}

void
GenParser::skipWord()
{
    skipWord(defSeps);
}

void 
GenParser::skipBytes(size_t n)
{
    start = end = end + n;
}

std::string
GenParser::readString(size_t size)
{
    std::string ret(file.begin() + start, file.begin() + start + size);
    start = end = end + size;
    return ret;
}

u8
GenParser::read8()
{
    auto ret = readTypeBytes<u8>(file, start);
    start = end = end + 1;
    return ret;
}

u16
GenParser::read16()
{
    auto ret = readTypeBytes<u16>(file, start);
    start = end = end + 2;
    return ret;
}

u32
GenParser::read32()
{
    auto ret = readTypeBytes<u32>(file, start);
    start = end = end + 4;
    return ret;
}

u64
GenParser::read64()
{
    auto ret = readTypeBytes<u64>(file, start);
    start = end = end + 8;
    return ret;
}

void
GenParser::setPos(size_t p)
{
    start = end = p;
}

bool
GenParser::finished()
{
    return start >= size();
}

bool
GenParser::isSeparator(char c, std::string_view separotors)
{
    if (!file[c])
        return false;

    for (char i : separotors)
        if (i == c)
            return true;

    return false;
}

void
GenParser::skipWhiteSpace()
{
    while (file[end] && (file[end] == ' ' || file[end] == '\n'))
        end++;

    start = end;
}
