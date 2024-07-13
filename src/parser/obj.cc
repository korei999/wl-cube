#include "obj.hh"

namespace parser
{

WaveFrontObj::WaveFrontObj(std::string_view defaultSeparators)
    : defSeps(defaultSeparators) {}

WaveFrontObj::WaveFrontObj(std::string_view path, std::string_view defaultSeparators)
    : defSeps(defaultSeparators)
{
    loadFile(path);
}

void
WaveFrontObj::nextWord(std::string_view separators)
{
    while (file[end] && !isSeparator(file[end], separators))
        end++;

    word = std::string_view(file).substr(start, end - start);
    start = end = end + 1;
}

void
WaveFrontObj::nextWord()
{
    nextWord(defSeps);
}

void
WaveFrontObj::skipWord(std::string_view separators)
{
    while (file[end] && !isSeparator(file[end], separators))
        end++;

    start = end = end + 1;
}

void
WaveFrontObj::skipWord()
{
    skipWord(defSeps);
}

bool
WaveFrontObj::isSeparator(char c, std::string_view separotors)
{
    if (!file[c])
        return false;

    for (char i : separotors)
        if (i == c)
            return true;

    return false;
}

void
WaveFrontObj::skipWhiteSpace()
{
    while (file[end] && (file[end] == ' ' || file[end] == '\n' || file[end] == '\r' || file[end] == '\t'))
        end++;

    start = end;
}

f32
WaveFrontObj::wordToFloat()
{
    f32 f = 0.0f;
    auto what = std::from_chars(word.data(), word.data() + word.size(), f);

    return f;
}

int
WaveFrontObj::wordToInt()
{
    int i = 0;

    if (word.size() != 0)
        auto what = std::from_chars(word.data(), word.data() + word.size(), i);

    return i;
}

} /* namespace parser */

