#include "headers/texture.hh"
#include "headers/utils.hh"

/* create with new, because it's must not be automatically destroyed prior to texture destruction */
/* silince the addres sanitizer */
auto* Texture::loadedTex = new std::unordered_map<u64, Texture*>;

Texture::Texture(std::string_view path, bool flip, GLint texMode)
{
    loadBMP(path, flip, texMode);
}

Texture::~Texture()
{
    if (this->id != 0)
    {
        // LOG(OK, "{}: id: {}, use_count: {}\n", this->texPath, this->id, this->idOwnersCounter.use_count());
        if (this->idOwnersCounter.use_count() == 1) /* one reference means that we are the only owner */
        {
            LOG(OK, "\ttexure '{}': id: '{}' deleted\n", this->texPath, this->id);
            auto found = loadedTex->find(hashFNV(this->texPath));
            loadedTex->erase(found);
            glDeleteTextures(1, &id);
        }
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
Texture::loadBMP(std::string_view path, bool flip, GLint texMode, WlClient* c)
{
    if (this->id != 0)
    {
        LOG(WARNING, "already set with id '{}'\n", this->id);
        return;
    }

    auto inserted = loadedTex->insert({hashFNV(path), this});

    if (!inserted.second)
    {
        LOG(WARNING, "texture '{}' is already loaded with id '{}', setting '{}' to this->id\n", path, inserted.first->second->id, inserted.first->second->id);
        this->idOwnersCounter = inserted.first->second->idOwnersCounter;
        this->id = *this->idOwnersCounter.get();
        this->texPath = path;
        return;
    }

    u32 imageDataAddress;
    s32 width;
    s32 height;
    u32 nPixels;
    u16 bitDepth;
    u8 byteDepth;

    LOG(OK, "loading '{}' bitmap...\n", path);

    Parser bmp(path, "", 0);
    auto BM = bmp.readString(2);

    if (BM != "BM")
        LOG(FATAL, "BM: {}, bmp file should have 'BM' as first 2 bytes\n", BM);

    bmp.skipBytes(8);
    imageDataAddress = bmp.read32();
    LOG(OK, "imageDataAddress: {}\n", imageDataAddress);

    bmp.skipBytes(4);
    width = bmp.read32();
    height = bmp.read32();
    LOG(OK, "width: {}, height: {}\n", width, height);

    [[maybe_unused]] auto colorPlane = bmp.read16();
    LOG(OK, "colorPlane: {}\n", colorPlane);

    GLint format = GL_RGB;
    bitDepth = bmp.read16();
    LOG(OK, "bitDepth: {}\n", bitDepth);

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

    bitDepth = 32;  /* forcing RBGA */
    nPixels = width * height;
    byteDepth = bitDepth / 8;
    LOG(OK, "nPixels: {}, byteDepth: {}\n", nPixels, byteDepth);
    std::vector<u8> pixels(nPixels * byteDepth);

    bmp.setPos(imageDataAddress);
    LOG(OK, "pos: {}, size: {}\n", bmp.start, bmp.size() - bmp.start);

    if (format == GL_RGBA)
        flipCpyBGRAtoRGBA((u8*)pixels.data(), (u8*)&bmp[bmp.start], width, height, flip);
    else
    {
        /* setting each alpha with 1.0 */
        flipCpyBGRtoRGBA((u8*)pixels.data(), (u8*)&bmp[bmp.start], width, height, flip);
        format = GL_RGBA;
    }

    setTexture((u8*)pixels.data(), texMode, format, width, height, c);

    LOG(OK, "{}: id: {}, texMode: {}\n", path, this->id, format);
    this->texPath = path;
}

void
Texture::bind(GLint glTexture)
{
    glActiveTexture(glTexture);
    glBindTexture(GL_TEXTURE_2D, this->id);
}

void
Texture::setTexture(u8* data, GLint texMode, GLint format, GLsizei width, GLsizei height, WlClient* c)
{
    std::lock_guard lock(glContextMtx);
    c->bindGlContext();

    glGenTextures(1, &this->id);
    glBindTexture(GL_TEXTURE_2D, this->id);
    /* set the texture wrapping parameters */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texMode);
    /* set texture filtering parameters */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    /* load image, create texture and generate mipmaps */
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    /* set the reference counter for each new texture */
    idOwnersCounter = std::make_shared<GLuint>(this->id);

    c->unbindGlContext();
}

CubeMapProjections::CubeMapProjections(const m4 proj, const v3 pos)
    : tms( 
        proj * m4LookAt(pos, pos + v3( 1, 0, 0), v3(0,-1, 0)),
        proj * m4LookAt(pos, pos + v3(-1, 0, 0), v3(0,-1, 0)),
        proj * m4LookAt(pos, pos + v3( 0, 1, 0), v3(0, 0, 1)),
        proj * m4LookAt(pos, pos + v3( 0,-1, 0), v3(0, 0,-1)),
        proj * m4LookAt(pos, pos + v3( 0, 0, 1), v3(0,-1, 0)),
        proj * m4LookAt(pos, pos + v3( 0, 0,-1), v3(0,-1, 0))
    ) {}

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
