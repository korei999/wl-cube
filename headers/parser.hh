#pragma once
#include "utils.hh"

struct Parser
{
    std::string word;

    std::string defSeps;
    std::vector<char> file;
    size_t start;
    size_t end;

    Parser(std::string_view defaultSeparators);
    Parser(std::string_view path, std::string_view defaultSeparators, size_t addZeroBytes = 1);

    char& operator[](size_t i) { return file[i]; };

    void loadFile(std::string_view path, size_t addZeroBytes = 1);
    void nextWord(std::string_view separators);
    void nextWord();
    void skipWord(std::string_view separators);
    void skipWord();
    void skipBytes(size_t n);
    std::string readString(size_t size);
    u8 read8();
    u16 read16();
    u32 read32();
    u64 read64();
    void setPos(size_t p);
    size_t size() const { return file.size(); };
    bool finished();
    bool isSeparator(char c, std::string_view separators);
};

template <typename Type>
#ifdef __linux__
__attribute__((no_sanitize("undefined"))) /* complains about unaligned pointers */
#endif
Type
readTypeBytes(const std::vector<char>& vec, size_t i)
{
    return *(Type*)&vec[i];
}
