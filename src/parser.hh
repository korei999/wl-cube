#pragma once
#include "utils.hh"

struct GenParser
{
    std::string word;
    std::string defSeps;
    std::vector<char> file;
    size_t start;
    size_t end;

    GenParser(std::string_view defaultSeparators);
    GenParser(std::string_view path, std::string_view defaultSeparators, size_t addZeroBytes = 1);

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
    void skipWhiteSpace();
};

template <typename Type>
#ifdef __linux__
__attribute__((no_sanitize("undefined"))) /* complains about unaligned pointers */
#endif
Type
readTypeBytes(std::vector<char>& vec, size_t i)
{
    return *reinterpret_cast<Type*>(&vec[i]);
}
