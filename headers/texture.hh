#pragma once
#include "gmath.hh"
#include "ultratypes.h"
#include "utils.hh"
#include "app.hh"
#include "gl.hh"

#include <string_view>
// #include <unordered_map>

enum TexType : int
{
    diffuse = 0,
    normal
};

struct Texture
{
    GLuint id = 0;
    TexType type;

    /* TODO: make some sort of shared ownership for same assets */
    std::string texPath;

    Texture() = default;
    Texture(std::string_view path, TexType type, bool flip = false, GLint texMode = GL_MIRRORED_REPEAT);
    ~Texture();

    void loadBMP(std::string_view path, TexType type, bool flip = false, GLint texMode = GL_MIRRORED_REPEAT, App* c = nullptr);
    void bind(GLint glTexture);

private:
    // static std::unordered_map<u64, Texture*>* loadedTex;

    void setTexture(u8* data, GLint texMode, GLint format, GLsizei width, GLsizei height, App* c);
};

struct ShadowMap
{
    GLuint fbo;
    GLuint tex;
    int width;
    int height;
};

struct CubeMap : ShadowMap
{
};

struct CubeMapProjections
{
    m4 tms[6];

    CubeMapProjections(const m4 projection, const v3 position);

    m4& operator[](size_t i) { return tms[i]; }
};

ShadowMap createShadowMap(const int width, const int height);
CubeMap createCubeShadowMap(const int width, const int height);
