#include "headers/utils.hh"

#include <vector>
#include <fstream>

GLenum glLastErrorCode = 0;
EGLint eglLastErrorCode = EGL_SUCCESS;

std::vector<char>
loadFileToStr(const std::string_view path, size_t addBytes)
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
