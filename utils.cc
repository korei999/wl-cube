#include "headers/utils.hh"

#include <fstream>
#include <chrono>

GLenum glLastErrorCode = 0;
EGLint eglLastErrorCode = EGL_SUCCESS;

std::vector<char>
loadFile(std::string_view path, size_t addBytes)
{
    std::ifstream file(path, std::ios::ate | std::ios::binary);
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
getTimeNow()
{
    typedef std::chrono::high_resolution_clock Time;
    typedef std::chrono::duration<f64> fsec;

    fsec fs = Time::now().time_since_epoch();
    return fs.count();
}

Parser::Parser(std::string_view path, std::string_view defaultSeparators, size_t addBytes)
    : defSeps(defaultSeparators)
{
    file = loadFile(path, addBytes);
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
