#pragma once
#include "gmath.hh"
#include "ultratypes.h"
#include "app.hh"
#include "gl/gl.hh"

#include <string>

enum TEX_TYPE : int
{
    DIFFUSE = 0,
    NORMAL
};

struct Texture
{
    GLuint id = 0;
    TEX_TYPE type;

    /* TODO: make some sort of shared ownership for same assets */
    std::string texPath;

    Texture() = default;
    Texture(std::string_view path, TEX_TYPE type, bool flip, GLint texMode, App* c);
    ~Texture();

    void loadBMP(std::string_view path, TEX_TYPE type, bool flip, GLint texMode, App* c);
    void bind(GLint glTexture);

private:
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
void flipCpyBGRAtoRGBA(u8* dest, u8* src, int width, int height, bool vertFlip);
void flipCpyBGRtoRGB(u8* dest, u8* src, int width, int height, bool vertFlip);
void flipCpyBGRtoRGBA(u8* dest, u8* src, int width, int height, bool vertFlip);
