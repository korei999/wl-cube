#include "headers/texture.hh"
#include "headers/utils.hh"

/* static placeholder */
std::unordered_map<std::string_view, Texture*> Texture::loadedTex;

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

Texture::Texture(std::string_view path, bool flip, GLint texMode)
{
    loadBMP(path, flip, texMode);
}

Texture::~Texture()
{
    if (this->id != 0)
    {
        LOG(OK, "id: {}, use_count: {}\n", this->id, this->idOwnersCounter.use_count());
        if (this->idOwnersCounter.use_count() == 1) /* one reference means that we are the only owner */
        {
            LOG(OK, "texure '{}' deleted\n", this->id);
            D( glDeleteTextures(1, &id) );
        }
    }
}

void
Texture::loadBMP(std::string_view path, bool flip, GLint texMode)
{
    if (this->id != 0)
    {
        LOG(WARNING, "already set with id '{}'\n", this->id);
        return;
    }

    auto inserted = loadedTex.insert({path, {}});

    if (!inserted.second)
    {
        LOG(WARNING, "texture '{}' is already loaded with id '{}', setting '{}' to this->id\n", path, inserted.first->second->id, inserted.first->second->id);
        this->idOwnersCounter = inserted.first->second->idOwnersCounter;
        this->id = *this->idOwnersCounter.get();
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

    [[maybe_unused]] auto bitsPerPixel = bmp.read16();
    LOG(OK, "bitsPerPixel: {}\n", bitsPerPixel);

    nPixels = width * height;
    byteDepth = bitDepth / 8;
    LOG(OK, "nPixels: {}, byteDepth: {}\n", nPixels, byteDepth);
    std::vector<u8> pixels(nPixels * byteDepth);

    bmp.setPos(imageDataAddress);
    LOG(OK, "pos: {}, size: {}\n", bmp.start, bmp.size() - bmp.start);

    if (format == GL_RGBA)
        flipCpyBGRAtoRGBA((u8*)pixels.data(), (u8*)&bmp[bmp.start], width, height, flip);
    else
        flipCpyBGRtoRGB((u8*)pixels.data(), (u8*)&bmp[bmp.start], width, height, flip);

    setTexture((u8*)pixels.data(), texMode, format, width, height);

    LOG(OK, "id: {}\n", this->id);
    inserted.first->second = this;
}

void
Texture::use()
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->id);
}

void
Texture::setTexture(u8* data, GLint texMode, GLint format, GLsizei width, GLsizei height)
{
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
}
