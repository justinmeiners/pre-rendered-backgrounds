#ifndef TEXTURE_H
#define TEXTURE_H

#include <stddef.h>


/*
 xcrun --sdk iphoneos texturetool -e PVRTC --channel-weighting-perceptual -m --bits-per-pixel-4 -f pvr /Users/justin/repos/personal/3d-adventure/data/levels/plant_1/chunk0.tga -o /Users/justin/repos/personal/3d-adventure/data/levels/plant_1/chunk0.pvr

 */


/* 
 Texture Atlas Info
 
 Mipmap Bleeding
 --------------------------
 - When generating mipmaps, neighbor texels may be sampled outside an individual texture border.
 - According to Nvidia paper, "Improve Batching Using Atlases", and verified by my GPU profiler. The NVIDIA texture tools (nvcompress to DDS) will protect power of two
aligned textures when using a box filter.
 - My Graphics driver also appears to generate correct mips with this same filter.
 - As long as I use the DDS tool this won't be a problem, otherwise I may have to custom make them.
 
 TODO: Does PVRTC generate correct mips?
 
 
 Filtering
 --------------------------
 - Linear min and mag filtering still causes neighbors to be samples.
 - It appears that by using GL_LINEAR for the mag filter and GL_NEAREST_MIPMAP_LINEAR the problem is averted (with correct mips).
 
 GL_NEAREST_MIPMAP_LINEAR
 
 "Chooses the two mipmaps that most closely match the size of the pixel
 being textured and uses the GL_NEAREST criterion
 (the texture element nearest to the center of the pixel)
 to produce a texture value from each mipmap.
 The final texture value is a weighted average of those two values."
 
 - https://www.khronos.org/opengles/sdk/docs/man/xhtml/glTexParameter.xml
 
 */

#define TEXTURE_STB 1

/* "OpenGL ES 3.0 also supports the ETC2 and EAC compressed texture formats; however, PVRTC textures are recommended on iOS devices." */


typedef enum
{
    kTextureFlagNone = 0,
    kTextureFlagAtlas = 1 << 0,
    kTextureFlagMipmap = 1 << 1,
    
    kTextureFlagDefault = kTextureFlagMipmap,
} TextureFlags;

typedef enum
{
    kTextureFormatRGBA = 0,
    kTextureFormatRGB,
    kTextureFormatGray,
    kTextureFormatDxt1, /* https://en.wikipedia.org/wiki/S3_Texture_Compression */
    kTextureFormatDxt3,
    kTextureFormatDxt5,
    kTextureFormatPvr2,
    kTextureFormatPvr2a, /* alpha variation */
    kTextureFormatPvr4,
    kTextureFormatPvr4a,  /* alpha variation */
    kTextureFormatUnknown,
} TextureFormat;

typedef enum
{
    kTexture2D = 0,
    kTextureCube,
} TextureType;

typedef enum
{
    kTextureCubeFaceRight = 0, // X+
    kTextureCubeFaceLeft, // X-
    kTextureCubeFaceTop, // Y+
    kTextureCubeFaceBottom, // Y-
    kTextureCubeFaceBack, // Z+
    kTextureCubeFaceFront, // Z-
    kTextureCubeFaceCount
} TextureCubeFace;

#define TEXTURE_SUBIMAGE_MAX 16

typedef struct
{
    /* For Renderer */
    unsigned int gpuId;
    
    /* info about each mip in the mipmap chain */
    struct
    {
        size_t offset;
        size_t length;
    } subimageInfo[TEXTURE_SUBIMAGE_MAX];
    unsigned short subimageCount;

    unsigned short width;
    unsigned short height;
    
    /* This flag allows the texture's RAM to be deleted after uploading to VRAM. Defaults to true */
    int purgeable;
    
    unsigned char* data;
    size_t dataLength;
    TextureFormat format;
    TextureType type;
    TextureFlags flags;
} Texture;

extern int Texture_FromPath(Texture* texture, TextureFlags flags, const char* path);
extern void Texture_Shutdown(Texture* texture);

extern void Texture_Purge(Texture* texture);


#endif
