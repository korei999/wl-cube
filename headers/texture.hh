#pragma once
#include "ultratypes.h"

#include <GLES3/gl32.h>
#include <memory>
#include <string_view>
#include <unordered_map>

struct Texture
{
    GLuint id = 0;
    std::shared_ptr<GLuint> idOwnersCounter;
    std::string_view texPath;

    Texture() = default;
    Texture(std::string_view path, bool flip = false, GLint texMode = GL_MIRRORED_REPEAT);
    ~Texture();

    void loadBMP(std::string_view path, bool flip = false, GLint texMode = GL_MIRRORED_REPEAT);
    void bind(GLint glTexture);

private:
    static std::unordered_map<std::string_view, Texture*>* loadedTex;

    void setTexture(u8* data, GLint texMode, GLint format, GLsizei width, GLsizei height);
};
