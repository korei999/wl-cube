#include "headers/parser.hh"

Parser::Parser(std::string_view defaultSeparators)
    : defSeps(defaultSeparators) {}

Parser::Parser(std::string_view path, std::string_view defaultSeparators, size_t addZeroBytes)
    : defSeps(defaultSeparators)
{
    loadFile(path, addZeroBytes);
}

void 
Parser::loadFile(std::string_view path, size_t addZeroBytes)
{
    start = end = 0;
    file = loadFileToCharArray(path, addZeroBytes);
}

void
Parser::nextWord(std::string_view separators)
{
    while (file[end] && !isSeparator(file[end], separators))
        end++;

    word = {file.begin() + start, file.begin() + end};
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
