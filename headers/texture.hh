#pragma once
#include "ultratypes.h"

#include <GLES3/gl3.h>
#include <string_view>
#include <unordered_map>

struct Texture
{
    GLuint id = 0;
    static std::unordered_map<std::string_view, GLuint> loadedTex;

    void loadBMP(std::string_view path, bool flip = false, GLint texMode = GL_MIRRORED_REPEAT);
    void use();

private:
    bool duplicateCheck(std::string_view path);
    void setTexture(u8* data, GLint texMode, GLint format, GLsizei width, GLsizei height);
};
