#pragma once
#include "ultratypes.h"

#include <GLES3/gl3.h>
#include <memory>
#include <string_view>
#include <unordered_map>

struct Texture
{
    GLuint id = 0;
    std::shared_ptr<GLuint> idOwnersCounter;

    Texture() = default;
    Texture(std::string_view path, bool flip = false, GLint texMode = GL_MIRRORED_REPEAT);
    ~Texture();

    void loadBMP(std::string_view path, bool flip = false, GLint texMode = GL_MIRRORED_REPEAT);
    void use();

private:
    static std::unordered_map<std::string_view, Texture*> loadedTex;

    bool duplicateCheck(std::string_view path);
    void setTexture(u8* data, GLint texMode, GLint format, GLsizei width, GLsizei height);
};
