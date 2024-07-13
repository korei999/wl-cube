#pragma once
#include "utils.hh"

namespace parser
{

struct Binary
{
    std::string_view word;
    std::string file;
    size_t start;
    size_t end;

    Binary() = default;
    Binary(std::string_view path);

    char& operator[](size_t i) { return file[i]; };

    void loadFile(std::string_view path);
    void skipBytes(size_t n);
    std::string readString(size_t size);
    u8 read8();
    u16 read16();
    u32 read32();
    u64 read64();
    void setPos(size_t p);
    size_t size() const { return file.size(); };
    bool finished();
};

template <typename T>
#ifdef __linux__
__attribute__((no_sanitize("undefined"))) /* unaligned pointers */
#endif
T
readTypeBytes(std::string& vec, size_t i)
{
    return *reinterpret_cast<T*>(&vec[i]);
}

} /* namespace parser */
