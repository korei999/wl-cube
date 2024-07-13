#pragma once

#include "bin.hh"

namespace parser
{

struct WaveFrontObj : Binary
{
    std::string defSeps = " /\n\t\r";

    WaveFrontObj() = default;
    WaveFrontObj(std::string_view defaultSeparators = " /\n\t\r");
    WaveFrontObj(std::string_view path, std::string_view defaultSeparators = " /\n\t\r");

    void nextWord(std::string_view separators);
    void nextWord();
    void skipWord(std::string_view separators);
    void skipWord();
    bool isSeparator(char c, std::string_view separators);
    void skipWhiteSpace();
    f32 wordToFloat();
    int wordToInt();
};

} /* namespace parser */
