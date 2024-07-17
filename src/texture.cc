#include <emmintrin.h>

#include "texture.hh"
#include "parser/bin.hh"

Texture::Texture(std::string_view path, TEX_TYPE type, bool flip, GLint texMode, App* c)
{
    this->loadBMP(path, type, flip, texMode, c);
}

Texture::~Texture()
{
    if (this->id != 0)
    {
        glDeleteTextures(1, &this->id);
        // LOG(OK, "texture {}: '{}' deleted\n", this->id, this->texPath);
    }
}

/* Bitmap file format
 *
 * SECTION
 * Address:Bytes	Name
 *
 * HEADER:
 *	  0:	2		"BM" magic number
 *	  2:	4		file size
 *	  6:	4		junk
 *	 10:	4		Starting address of image data
 * BITMAP HEADER:
 *	 14:	4		header size
 *	 18:	4		width  (signed)
 *	 22:	4		height (signed)
 *	 26:	2		Number of color planes
 *	 28:	2		Bits per pixel
 *	[...]
 * [OPTIONAL COLOR PALETTE, NOT PRESENT IN 32 BIT BITMAPS]
 * BITMAP DATA:
 *	DATA:	X	Pixels
 */

void
Texture::loadBMP(std::string_view path, TEX_TYPE type, bool flip, GLint texMode, App* c)
{
    LOG(OK, "loading '{}' texture...\n", path);

    if (this->id != 0)
    {
        LOG(WARNING, "already set with id '{}'\n", this->id);
        return;
    }
    this->texPath = path;
    this->type = type;

    u32 imageDataAddress;
    s32 width;
    s32 height;
    u32 nPixels;
    u16 bitDepth;
    u8 byteDepth;

    parser::Binary p(path);
    auto BM = p.readString(2);

    if (BM != "BM")
        LOG(FATAL, "BM: {}, bmp file should have 'BM' as first 2 bytes\n", BM);

    p.skipBytes(8);
    imageDataAddress = p.read32();

#ifdef TEXTURE
    LOG(OK, "imageDataAddress: {}\n", imageDataAddress);
#endif

    p.skipBytes(4);
    width = p.read32();
    height = p.read32();
#ifdef TEXTURE
    LOG(OK, "width: {}, height: {}\n", width, height);
#endif

    [[maybe_unused]] auto colorPlane = p.read16();
#ifdef TEXTURE
    LOG(OK, "colorPlane: {}\n", colorPlane);
#endif

    GLint format = GL_RGB;
    bitDepth = p.read16();
#ifdef TEXTURE
    LOG(OK, "bitDepth: {}\n", bitDepth);
#endif

    switch (bitDepth)
    {
        case 24:
            format = GL_RGB;
            break;

        case 32:
            format = GL_RGBA;
            break;

        default:
            LOG(WARNING, "support only for 32 and 24 bit bmp's, read '{}', setting to GL_RGB\n", bitDepth);
            break;
    }

    bitDepth = 32; /* use RGBA anyway */
    nPixels = width * height;
    byteDepth = bitDepth / 8;
#ifdef TEXTURE
    LOG(OK, "nPixels: {}, byteDepth: {}\n", nPixels, byteDepth);
#endif
    std::vector<u8> pixels(nPixels * byteDepth);

    p.setPos(imageDataAddress);
#ifdef TEXTURE
    LOG(OK, "pos: {}, size: {}\n", bmp.start, bmp.size() - bmp.start);
#endif

    switch (format)
    {
        default:
        case GL_RGB:
            flipCpyBGRtoRGBA(pixels.data(), reinterpret_cast<u8*>(&p[p.start]), width, height, flip);
            format = GL_RGBA;
            break;

        case GL_RGBA:
            flipCpyBGRAtoRGBA(pixels.data(), reinterpret_cast<u8*>(&p[p.start]), width, height, flip);
            break;
    }

    setTexture(pixels.data(), texMode, format, width, height, c);

#ifdef TEXTURE
    LOG(OK, "{}: id: {}, texMode: {}\n", path, this->id, format);
#endif
}

void
Texture::bind(GLint glTexture)
{
    glActiveTexture(glTexture);
    glBindTexture(GL_TEXTURE_2D, this->id);
}

void
Texture::setTexture(u8* pData, GLint texMode, GLint format, GLsizei width, GLsizei height, App* c)
{
    std::lock_guard lock(gl::mtxGlContext);
    c->bindGlContext();

    glGenTextures(1, &this->id);
    glBindTexture(GL_TEXTURE_2D, this->id);
    /* set the texture wrapping parameters */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texMode);
    /* set texture filtering parameters */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    /* load image, create texture and generate mipmaps */
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, pData);
    glGenerateMipmap(GL_TEXTURE_2D);

    c->unbindGlContext();
}

CubeMapProjections::CubeMapProjections(const m4 proj, const v3 pos)
    : tms{
        proj * m4LookAt(pos, pos + v3( 1, 0, 0), v3(0,-1, 0)),
        proj * m4LookAt(pos, pos + v3(-1, 0, 0), v3(0,-1, 0)),
        proj * m4LookAt(pos, pos + v3( 0, 1, 0), v3(0, 0, 1)),
        proj * m4LookAt(pos, pos + v3( 0,-1, 0), v3(0, 0,-1)),
        proj * m4LookAt(pos, pos + v3( 0, 0, 1), v3(0,-1, 0)),
        proj * m4LookAt(pos, pos + v3( 0, 0,-1), v3(0,-1, 0))
    } {}

ShadowMap
createShadowMap(const int width, const int height)
{
    GLenum none = GL_NONE;
    ShadowMap res {};
    res.width = width;
    res.height = height;

    glGenTextures(1, &res.tex);
    glBindTexture(GL_TEXTURE_2D, res.tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	f32 borderColor[] {1.0, 1.0, 1.0, 1.0};
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);

    GLint defFramebuffer = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &defFramebuffer);
    /* set up fbo */
    glGenFramebuffers(1, &res.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, res.fbo);

    glDrawBuffers(1, &none);
    glReadBuffer(GL_NONE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, res.tex, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, res.tex);

    if (GL_FRAMEBUFFER_COMPLETE != glCheckFramebufferStatus(GL_FRAMEBUFFER))
        LOG(FATAL, "glCheckFramebufferStatus != GL_FRAMEBUFFER_COMPLETE\n"); 

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return res;
}

CubeMap
createCubeShadowMap(const int width, const int height)
{
    GLuint depthCubeMap;
    glGenTextures(1, &depthCubeMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap);

    for (GLuint i = 0; i < 6; i++)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                     0,
                     GL_DEPTH_COMPONENT,
                     width,
                     height,
                     0,
                     GL_DEPTH_COMPONENT,
                     GL_FLOAT,
                     nullptr);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubeMap, 0);
    GLenum none = GL_NONE;
    glDrawBuffers(1, &none);
    glReadBuffer(GL_NONE);

    if (GL_FRAMEBUFFER_COMPLETE != glCheckFramebufferStatus(GL_FRAMEBUFFER))
        LOG(FATAL, "glCheckFramebufferStatus != GL_FRAMEBUFFER_COMPLETE\n"); 

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return {fbo, depthCubeMap, width, height};
}

/* complains about unaligned address */
void
flipCpyBGRAtoRGBA(u8* dest, u8* src, int width, int height, bool vertFlip)
{
    int f = vertFlip ? -(height - 1) : 0;
    int inc = vertFlip ? 2 : 0;

    u32* d = reinterpret_cast<u32*>(dest);
    u32* s = reinterpret_cast<u32*>(src);

    auto swapRedBlueBits = [](u32 col) -> u32 {
        u32 r = col & 0x00'ff'00'00;
        u32 b = col & 0x00'00'00'ff;
        return (col & 0xff'00'ff'00) | (r >> (4*4)) | (b << (4*4));
    };

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x += 4)
        {
            u32 colorsPack[4];
            for (size_t i = 0; i < std::size(colorsPack); i++)
                colorsPack[i] = swapRedBlueBits(s[y*width + x + i]);

            auto _dest = reinterpret_cast<__m128i_u*>(&d[(y-f)*width + x]);
            _mm_storeu_si128(_dest, *reinterpret_cast<__m128i*>(colorsPack));
        }

        f += inc;
    }
};

void
flipCpyBGRtoRGB(u8* dest, u8* src, int width, int height, bool vertFlip)
{
    int f = vertFlip ? -(height - 1) : 0;
    int inc = vertFlip ? 2 : 0;

    constexpr int nComponents = 3;
    width = width * nComponents;

    auto at = [=](int x, int y, int z) -> int {
        return y*width + x + z;
    };

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x += nComponents)
        {
            dest[at(x, y-f, 0)] = src[at(x, y, 2)];
            dest[at(x, y-f, 1)] = src[at(x, y, 1)];
            dest[at(x, y-f, 2)] = src[at(x, y, 0)];
        }
        f += inc;
    }
};

void
flipCpyBGRtoRGBA(u8* dest, u8* src, int width, int height, bool vertFlip)
{
    int f = vertFlip ? -(height - 1) : 0;
    int inc = vertFlip ? 2 : 0;

    constexpr int rgbComp = 3;
    constexpr int rgbaComp = 4;

    int rgbWidth = width * rgbComp;
    int rgbaWidth = width * rgbaComp;

    auto at = [](int width, int x, int y, int z) -> int {
        return y*width + x + z;
    };

    for (int y = 0; y < height; y++)
    {
        for (int xSrc = 0, xDest = 0; xSrc < rgbWidth; xSrc += rgbComp, xDest += rgbaComp)
        {
            dest[at(rgbaWidth, xDest, y-f, 0)] = src[at(rgbWidth, xSrc, y, 2)];
            dest[at(rgbaWidth, xDest, y-f, 1)] = src[at(rgbWidth, xSrc, y, 1)];
            dest[at(rgbaWidth, xDest, y-f, 2)] = src[at(rgbWidth, xSrc, y, 0)];
            dest[at(rgbaWidth, xDest, y-f, 3)] = 0xff;
        }
        f += inc;
    }
};
