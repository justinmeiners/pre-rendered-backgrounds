

#include "gl_3.h"
#include "gl_compat.h"
#include "gl_prog.h"
#include "utils.h"
#include <string.h>


/*
 I had some painful bugs working with VBOs and IBOs.
 Apparently IBOs are stored in the VAO state but VBOs are not.
 glVertexAttribPointer still keeps a reference to the VBO however.
 This has the following consequences:
 - rendering requires nothing to be bound but the VAO
 - updating the IBO requires the VAO to be bound
 - updating the VBO requires itself to be bound

 */

#define VBO_OFFSET(i) ((char *)NULL + (i))

enum
{
    kGl2ProgramStaticLit,
    kGl2ProgramHint,
    kGl2ProgramSkelLit,
    kGl2ProgramSkelSolid,
    kGl2ProgramPart,
    kGl2ProgramGui,
    kGl2ProgramBg,
    kGl2ProgramCount,
} Gl2ShaderType;

enum
{
    kProgLocModel,
    kProgLocView,
    kProgLocProjection,
    kProgLocAlbedo,
    kProgLocNormal,
    kProgLocSpecular,
    kProgLocGloss,
    kProgLocEnvMap,
    kProgLocDepth,
    
    kProgLocJointRotations,
    kProgLocJointOrigins,
    kProgLocCamPosition,
    
    kProgLocLightPositions,
    
    kProgLocColor,

    kProgLocAtlas,
    kProgLocControlPoints,
    kProgLocOrigin,
    kProgLocPartCount,
    kProgLocPartEffect,
    kProgLocTime,
};


typedef struct
{
    GlProg programs[kGl2ProgramCount];
    
    int partVao;
    int partVbo;
    
} Gl2Context;


static int Gl_UploadSkelSkin(Renderer* gl, SkelSkin* skin)
{
    GLuint vboId, vaoId;
    
    glGenVertexArrays(1, &vaoId);
    glBindVertexArray(vaoId);
    
    glGenBuffers(1, &vboId);
    glBindBuffer(GL_ARRAY_BUFFER, vboId);
    
    skin->vboGpuId = vboId;
    skin->vaoGpuId = vaoId;
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(SkelSkinVert) * skin->vertCount, skin->verts, GL_STATIC_DRAW);
    
    size_t offset = 0;
    
    glEnableVertexAttribArray(kGlAttribNormal);
    glVertexAttribPointer(kGlAttribNormal, 3, GL_FLOAT, GL_FALSE, sizeof(SkelSkinVert), VBO_OFFSET(offset));
    offset += sizeof(Vec3);
    
    glEnableVertexAttribArray(kGlAttribTangent);
    glVertexAttribPointer(kGlAttribTangent, 3, GL_FLOAT, GL_FALSE, sizeof(SkelSkinVert), VBO_OFFSET(offset));
    offset += sizeof(Vec3);
    
    glEnableVertexAttribArray(kGlAttribUv0);
    glVertexAttribPointer(kGlAttribUv0, 2, GL_UNSIGNED_SHORT, GL_TRUE, sizeof(SkelSkinVert), VBO_OFFSET(offset));
    offset += sizeof(SkelSkinVertUv);
    
    for (int i = 0; i < SKEL_WEIGHTS_PER_VERT; ++i)
    {
        glEnableVertexAttribArray(kGlAttribWeight0 + i);
        glVertexAttribPointer(kGlAttribWeight0 + i, 4, GL_FLOAT, GL_FALSE, sizeof(SkelSkinVert), VBO_OFFSET(offset));
        offset += sizeof(Vec4);
    }
    
    glEnableVertexAttribArray(kGlAttribWeightJoints);
    glVertexAttribPointer(kGlAttribWeightJoints, SKEL_WEIGHTS_PER_VERT, GL_SHORT, GL_FALSE, sizeof(SkelSkinVert), VBO_OFFSET(offset));
    //offset += sizeof(short) * SKEL_WEIGHTS_PER_VERT;
    
    if (skin->purgeable)
    {
        SkelSkin_Purge(skin);
    }

    return 1;
}

static int Gl_CleanupSkelSkin(Renderer* gl, SkelSkin* skin)
{
    glDeleteBuffers(1, &skin->vboGpuId);
    glDeleteVertexArrays(1, &skin->vaoGpuId);

    return 1;
}

static int Gl_UploadMesh(Renderer* gl, StaticMesh* mesh)
{
    GLuint vboId, vaoId;
    
    glGenVertexArrays(1, &vaoId);
    glBindVertexArray(vaoId);
    
    glGenBuffers(1, &vboId);
    glBindBuffer(GL_ARRAY_BUFFER, vboId);
    
    mesh->vboGpuId = vboId;
    mesh->vaoGpuId = vaoId;
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(StaticMeshVert) * mesh->vertCount, mesh->verts, GL_STATIC_DRAW);

    size_t offset = 0;
    
    glEnableVertexAttribArray(kGlAttribVertex);
    glVertexAttribPointer(kGlAttribVertex, 3, GL_FLOAT, GL_FALSE, sizeof(StaticMeshVert), VBO_OFFSET(offset));
    offset += sizeof(Vec3);
    
    glEnableVertexAttribArray(kGlAttribNormal);
    glVertexAttribPointer(kGlAttribNormal, 3, GL_FLOAT, GL_FALSE, sizeof(StaticMeshVert), VBO_OFFSET(offset));
    offset += sizeof(Vec3);
    
    int i;
    for (i = 0; i < mesh->uvChannelCount; ++i)
    {
        glEnableVertexAttribArray(kGlAttribUv0 + i);
        glVertexAttribPointer(kGlAttribUv0 + i, 2, GL_UNSIGNED_SHORT, GL_TRUE, sizeof(StaticMeshVert), VBO_OFFSET(offset));
        offset += sizeof(StaticMeshVertUv);
    }
    
    if (mesh->purgeable)
        StaticMesh_Purge(mesh);
    
    return 1;
}

static int Gl_CleanupMesh(Renderer* gl, StaticMesh* mesh)
{
    glDeleteVertexArrays(1, &mesh->vaoGpuId);
    glDeleteBuffers(1, &mesh->vboGpuId);
    
    return 1;
}

static GLenum Gl_GetTextureFormat(TextureFormat textureFormat)
{
    GLenum format;
    switch (textureFormat)
    {
#ifdef GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
        case kTextureFormatDxt1:
            format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
            break;
        case kTextureFormatDxt3:
            format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
            break;
        case kTextureFormatDxt5:
            format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
            break;
#endif
#ifdef GL_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT
        /* I had an issue where PVR compressed textures were appearing especially dark.
         It turns out I specifying the sRGB gamma corrected format instead of the standard RGB format */
        
        case kTextureFormatPvr2:
            format = GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
            break;
        case kTextureFormatPvr2a:
            format = GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
            break;
        case kTextureFormatPvr4:
            format = GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
            break;
        case kTextureFormatPvr4a:
            format = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
            break;
#endif
        case kTextureFormatRGB:
            format = GL_RGB8;
            break;
        case kTextureFormatRGBA:
            format = GL_RGBA8;
            break;
        case kTextureFormatGray:
            format = GL_RED;
            break;
        default:
            format = GL_RGBA;
            break;
    }

    return format;
}

static int Gl_UploadTextureCube(Renderer* gl, Texture* texture)
{
    if (texture->gpuId != 0)
    {
        GLuint gpuID = (GLuint)texture->gpuId;
        glDeleteTextures(1, &gpuID);
    }
    
    GLenum format = Gl_GetTextureFormat(texture->format);
    GLenum internalFormat = GL_RGB;
    
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
    
    for (int i = 0; i < kTextureCubeFaceCount; ++i)
    {
        unsigned char* data = texture->data + texture->subimageInfo[i].offset;
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, texture->width, texture->height, 0, internalFormat, GL_UNSIGNED_BYTE, data);
    }
    
    if (texture->purgeable)
    {
        Texture_Purge(texture);
        texture->data = NULL;
    }
    
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    
    texture->gpuId = tex;
    return 1;
}

static int Gl_UploadTexture2d(Renderer* gl, Texture* texture)
{
    if (texture->gpuId != 0)
    {
        GLuint gpuID = (GLuint)texture->gpuId;
        glDeleteTextures(1, &gpuID);
    }
    
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    
    GLenum format = Gl_GetTextureFormat(texture->format);
    
    if (texture->subimageCount > 1)
    {
        short width = texture->width;
        short height = texture->height;
        
        int i;
        for (i = 0; i < texture->subimageCount; ++i)
        {
            glCompressedTexImage2D(GL_TEXTURE_2D, i, format, width, height, 0, (GLint)texture->subimageInfo[i].length, texture->data + texture->subimageInfo[i].offset);
            width = MAX((width >> 1), 1);
            height = MAX((height >> 1), 1);
        }
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, i - 1);
    }
    else
    {
        GLenum internalFormat = GL_RGBA;
        if (texture->format == kTextureFormatRGB)
            internalFormat = GL_RGB;
        
        glTexImage2D(GL_TEXTURE_2D, 0, format, texture->width, texture->height, 0, internalFormat, GL_UNSIGNED_BYTE, texture->data);
        
        if (texture->flags & kTextureFlagMipmap)
            glGenerateMipmap(GL_TEXTURE_2D);
    }
    
    if (texture->flags & kTextureFlagMipmap)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,
                        GL_TEXTURE_MIN_FILTER,
                        GL_LINEAR_MIPMAP_LINEAR);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    texture->gpuId = tex;
    
    if (texture->purgeable)
    {
        Texture_Purge(texture);
        texture->data = NULL;
    }
    
    return 1;
}


static int Gl_UploadTexture(Renderer* gl, Texture* texture)
{
    if (texture->type == kTexture2D)
    {
        return Gl_UploadTexture2d(gl, texture);
    }
    else if (texture->type == kTextureCube)
    {
        return Gl_UploadTextureCube(gl, texture);
    }
    
    return 0;
}


static int Gl_CleanupTexture(Renderer* gl, Texture* texture)
{
    glDeleteTextures(1, &texture->gpuId);
    return 1;
}

static int Gl_PrepareBgBuffer(Renderer* gl, BgBuffer* buffer)
{
    Vec2 verts[4];
    verts[0] = Vec2_Create(-1.0f, -1.0f);
    verts[1] = Vec2_Create(1.0f, -1.0f);
    verts[2] = Vec2_Create(1.0f, 1.0f);
    verts[3] = Vec2_Create(-1.0f, 1.0f);
    
    unsigned short indicies[6];
    indicies[0] = 0;
    indicies[1] = 1;
    indicies[2] = 3;
    
    indicies[3] = 3;
    indicies[4] = 1;
    indicies[5] = 2;
    
    GLuint vao, vbo, ibo;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * 6, indicies, GL_STATIC_DRAW);
    
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(Vec2), verts, GL_STATIC_DRAW);
    
    size_t offset = 0;
    glEnableVertexAttribArray(kGlAttribVertex);
    glVertexAttribPointer(kGlAttribVertex, 2, GL_FLOAT, GL_FALSE, sizeof(Vec2), VBO_OFFSET(offset));
    
    buffer->vaoGpuId = vao;
    buffer->vboGpuId = vbo;
    buffer->iboGpuId = ibo;

    return 1;
}

static int Gl_PrepareHintBuffer(Renderer* gl, HintBuffer* buffer)
{
    GLuint vao, vbo, ibo;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * HINT_VERTS_MAX, NULL, GL_DYNAMIC_DRAW);
    
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, HINT_VERTS_MAX * sizeof(HintVert), NULL, GL_DYNAMIC_DRAW);
    
    size_t offset = 0;
    glEnableVertexAttribArray(kGlAttribVertex);
    glVertexAttribPointer(kGlAttribVertex, 3, GL_FLOAT, GL_FALSE, sizeof(HintVert), VBO_OFFSET(offset));
    offset += sizeof(Vec3);
    
    glEnableVertexAttribArray(kGlAttribColor);
    glVertexAttribPointer(kGlAttribColor, 3, GL_FLOAT, GL_FALSE, sizeof(HintVert), VBO_OFFSET(offset));
    //offset += sizeof(Vec3);
    
    buffer->vaoGpuId = vao;
    buffer->vboGpuId = vbo;
    buffer->iboGpuId = ibo;
    
    return 1;
}

static int Gl_CleanupHintBuffer(Renderer* gl, HintBuffer* buffer)
{
    glDeleteVertexArrays(1, &buffer->vaoGpuId);
    glDeleteBuffers(1, &buffer->iboGpuId);
    glDeleteBuffers(1, &buffer->vboGpuId);
    return 1;
}

static int Gl_PrepareGuiBuffer(Renderer* gl, GuiBuffer* buffer)
{
    GLuint vao, vbo, ibo;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * GUI_VERTS_MAX, 0, GL_DYNAMIC_DRAW);
    
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, GUI_VERTS_MAX * sizeof(GuiVert), 0, GL_DYNAMIC_DRAW);
    
    size_t offset = 0;
    glEnableVertexAttribArray(kGlAttribVertex);
    glVertexAttribPointer(kGlAttribVertex, 2, GL_FLOAT, GL_FALSE, sizeof(GuiVert), VBO_OFFSET(offset));
    offset += sizeof(Vec2);
    
    glEnableVertexAttribArray(kGlAttribUv0);
    glVertexAttribPointer(kGlAttribUv0, 2, GL_UNSIGNED_SHORT, GL_TRUE, sizeof(GuiVert), VBO_OFFSET(offset));
    offset += sizeof(GuiVertUv);
    
    glEnableVertexAttribArray(kGlAttribColor);
    glVertexAttribPointer(kGlAttribColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(GuiVert), VBO_OFFSET(offset));
    offset += sizeof(GuiVertColor);
    
    glEnableVertexAttribArray(kGlAttribAtlasId);
    glVertexAttribPointer(kGlAttribAtlasId, 1, GL_BYTE, GL_FALSE, sizeof(GuiVert), VBO_OFFSET(offset));

    buffer->vaoGpuId = vao;
    buffer->vboGpuId = vbo;
    buffer->iboGpuId = ibo;

    return 1;
}

static int Gl_CleanupGuiBuffer(Renderer* gl, GuiBuffer* buffer)
{
    glDeleteVertexArrays(1, &buffer->vaoGpuId);
    glDeleteBuffers(1, &buffer->iboGpuId);
    glDeleteBuffers(1, &buffer->vboGpuId);
    return 1;
}

static int Gl_CheckExtension(Renderer* gl, const char* extension, const char* extensionString)
{
    if (!extension || !extensionString)
        return 0;
    
    const char* p = extensionString;
    size_t extNameLen = strlen(extension);
    
    while (*p != '\0')
    {
        size_t n = strcspn(p, " ");
        
        if ((extNameLen == n) && (strncmp(extension, p, n) == 0)) {
            return 1;
        }
        p += (n + 1);
    }
    
    return 0;
}

static int Gl_CheckCompatibility(Renderer* gl, RendererLimits* limits)
{
    printf("vendor: %s\n", glGetString(GL_VENDOR));
    printf("renderer: %s\n", glGetString(GL_RENDERER));
    printf("version: %s\n", glGetString(GL_VERSION));

    const char* extensions = (const char*)glGetString(GL_EXTENSIONS);
    
    if (extensions)
    {
        printf("%s\n", extensions);
        if (Gl_CheckExtension(gl, "OES_vertex_array_object", extensions))
        {
            printf("missing OES_vertex_array_object\n");
            return 0;
        }
    }
    
    GLint attribs = 0;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &attribs);
    printf("vertex attribs: %i\n", attribs);
    
    
    GLint components = 0;
#if __APPLE__
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS, &components);
#else
	// from https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGet.xhtml
	// The value of GL_MAX_VERTEX_UNIFORM_VECTORS
	// is equal to the value of GL_MAX_VERTEX_UNIFORM_COMPONENTS
	// and must be at least 256.
	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &components);
#endif // __APPLE__
    printf("vertex uniform components: %i\n", components);
    
    limits->maxSkelJoints = components / 4;
    
    GLint maxTextureSize = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
    limits->maxTextureSize = maxTextureSize;
    return 1;
}

static int Gl_Init(Renderer* gl)
{
    Gl2Context* ctx = gl->context;
    
    if (!Gl_CheckCompatibility(gl, &gl->limits))
    {
        printf("hardware does not meet basic OpenGL requirements.\n");
        return 0;
    }
    
    glCullFace(GL_BACK);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_SCISSOR_TEST);
    
    // background shader
    // ------------------------------------
    
    char vertPath[MAX_OS_PATH];
    char fragPath[MAX_OS_PATH];
    
    Filepath_Append(vertPath, Filepath_DataPath(), "shaders/bg.vs");
    Filepath_Append(fragPath, Filepath_DataPath(), "shaders/bg.fs");
    
    GlProg* bg = &ctx->programs[kGl2ProgramBg];
    GlProg_InitWithPaths(bg, vertPath, fragPath);
    
    GlProg_BindAttrib(bg, kGlAttribVertex, "a_vertex");
    GlProg_Link(bg, 1);
    GlProg_MapUniformLoc(bg, "u_albedo", kProgLocAlbedo);
    GlProg_MapUniformLoc(bg, "u_depth", kProgLocDepth);

    // object shader
    // ------------------------------------
    
    Filepath_Append(vertPath, Filepath_DataPath(), "shaders/static_lit.vs");
    Filepath_Append(fragPath, Filepath_DataPath(), "shaders/static_lit.fs");
    
    GlProg* staticLit = ctx->programs + kGl2ProgramStaticLit;
    GlProg_InitWithPaths(staticLit, vertPath, fragPath);

    GlProg_BindAttrib(staticLit, kGlAttribVertex, "a_vertex");
    GlProg_BindAttrib(staticLit, kGlAttribNormal, "a_normal");
    GlProg_BindAttrib(staticLit, kGlAttribUv0, "a_uv0");
    GlProg_Link(staticLit, 1);
    
    GlProg_MapUniformLoc(staticLit, "u_model", kProgLocModel);
    GlProg_MapUniformLoc(staticLit, "u_view", kProgLocView);
    GlProg_MapUniformLoc(staticLit, "u_projection", kProgLocProjection);
    GlProg_MapUniformLoc(staticLit, "u_albedo", kProgLocAlbedo);
    GlProg_MapUniformLoc(staticLit, "u_specular", kProgLocSpecular);
    GlProg_MapUniformLoc(staticLit, "u_camPosition", kProgLocCamPosition);
    GlProg_MapUniformLoc(staticLit, "u_lightPositions[0]", kProgLocLightPositions);

    
    // Skeleton shader
    // ------------------------------------
    
    Filepath_Append(vertPath, Filepath_DataPath(), "shaders/skel_lit.vs");
    Filepath_Append(fragPath, Filepath_DataPath(), "shaders/skel_lit.fs");

    GlProg* skel = ctx->programs + kGl2ProgramSkelLit;
    GlProg_InitWithPaths(skel, vertPath, fragPath);

    GlProg_BindAttrib(skel, kGlAttribNormal, "a_normal");
    GlProg_BindAttrib(skel, kGlAttribTangent, "a_tangent");
    GlProg_BindAttrib(skel, kGlAttribUv0, "a_uv0");
    
    GlProg_BindAttrib(skel, kGlAttribWeightJoints, "a_weight_joints");
    
    GlProg_BindAttrib(skel, kGlAttribWeight0, "a_weight0");
    GlProg_BindAttrib(skel, kGlAttribWeight1, "a_weight1");
    GlProg_BindAttrib(skel, kGlAttribWeight2, "a_weight2");
    
    GlProg_Link(skel, 1);
    
    GlProg_MapUniformLoc(skel, "u_model", kProgLocModel);
    GlProg_MapUniformLoc(skel, "u_view", kProgLocView);
    GlProg_MapUniformLoc(skel, "u_projection", kProgLocProjection);
    
    GlProg_MapUniformLoc(skel, "u_albedo", kProgLocAlbedo);
    GlProg_MapUniformLoc(skel, "u_normal", kProgLocNormal);
    GlProg_MapUniformLoc(skel, "u_specular", kProgLocSpecular);
    GlProg_MapUniformLoc(skel, "u_gloss", kProgLocGloss);
    GlProg_MapUniformLoc(skel, "u_envMap", kProgLocEnvMap);

    GlProg_MapUniformLoc(skel, "u_camPosition", kProgLocCamPosition);
    GlProg_MapUniformLoc(skel, "u_lightPositions[0]", kProgLocLightPositions);

    GlProg_MapUniformLoc(skel, "u_jointRotations[0]", kProgLocJointRotations);
    GlProg_MapUniformLoc(skel, "u_jointOrigins[0]", kProgLocJointOrigins);
    
    // Skeleton solid shader
    // ------------------------------------
    Filepath_Append(vertPath, Filepath_DataPath(), "shaders/skel_solid.vs");
    Filepath_Append(fragPath, Filepath_DataPath(), "shaders/skel_solid.fs");
    
    GlProg* skelSolid = ctx->programs + kGl2ProgramSkelSolid;
    GlProg_InitWithPaths(skelSolid, vertPath, fragPath);
        
    GlProg_BindAttrib(skelSolid, kGlAttribWeightJoints, "a_weight_joints");
    
    GlProg_BindAttrib(skelSolid, kGlAttribWeight0, "a_weight0");
    GlProg_BindAttrib(skelSolid, kGlAttribWeight1, "a_weight1");
    GlProg_BindAttrib(skelSolid, kGlAttribWeight2, "a_weight2");
    
    GlProg_Link(skelSolid, 1);
    
    GlProg_MapUniformLoc(skelSolid, "u_model", kProgLocModel);
    GlProg_MapUniformLoc(skelSolid, "u_view", kProgLocView);
    GlProg_MapUniformLoc(skelSolid, "u_projection", kProgLocProjection);
    
    GlProg_MapUniformLoc(skelSolid, "u_jointRotations[0]", kProgLocJointRotations);
    GlProg_MapUniformLoc(skelSolid, "u_jointOrigins[0]", kProgLocJointOrigins);
    GlProg_MapUniformLoc(skelSolid, "u_color", kProgLocColor);

    // GUI shader
    // ------------------------------------

    Filepath_Append(vertPath, Filepath_DataPath(), "shaders/gui.vs");
    Filepath_Append(fragPath, Filepath_DataPath(), "shaders/gui.fs");

    GlProg* gui = &ctx->programs[kGl2ProgramGui];
    GlProg_InitWithPaths(gui, vertPath, fragPath);
    
    GlProg_BindAttrib(gui, kGlAttribVertex, "a_vertex");
    GlProg_BindAttrib(gui, kGlAttribColor, "a_color");
    GlProg_BindAttrib(gui, kGlAttribUv0, "a_uv0");
    GlProg_BindAttrib(gui, kGlAttribAtlasId, "a_atlas_id");

    GlProg_Link(gui, 1);
    
    GlProg_MapUniformLoc(gui, "u_projection", kProgLocProjection);
    GlProg_MapUniformLoc(gui, "u_atlas[0]", kProgLocAtlas);
    
    // Hint shader
    // ------------------------------------
    
    Filepath_Append(vertPath, Filepath_DataPath(), "shaders/hint.vs");
    Filepath_Append(fragPath, Filepath_DataPath(), "shaders/hint.fs");
    
    GlProg* hint = &ctx->programs[kGl2ProgramHint];
    GlProg_InitWithPaths(hint, vertPath, fragPath);
    
    GlProg_BindAttrib(hint, kGlAttribVertex, "a_vertex");
    GlProg_BindAttrib(hint, kGlAttribColor, "a_color");
    
    GlProg_Link(hint, 1);
    GlProg_MapUniformLoc(hint, "u_projection", kProgLocProjection);
    GlProg_MapUniformLoc(hint, "u_view", kProgLocView);

    return 1;
}

static void Gl_Shutdown(Renderer* gl)
{
    Gl2Context* ctx = gl->context;

    for (int i = 0; i < kGl2ProgramCount; ++ i)
    {
        GlProg_Shutdown(ctx->programs + i);
    }
}

static void Gl_UpdateBuffers(Renderer* gl,
                              const Engine* engine,
                              const HintBuffer* hintBuffer,
                              const GuiBuffer* guiBuffer)
{
    /* upload hint buffer data */
    glBindVertexArray(hintBuffer->vaoGpuId);
    
    glBindBuffer(GL_ARRAY_BUFFER, hintBuffer->vboGpuId);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(unsigned short) * hintBuffer->indexCount, hintBuffer->indicies);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(HintVert) * hintBuffer->vertCount, hintBuffer->verts);
    
    
    glBindVertexArray(guiBuffer->vaoGpuId);

    // upload gui buffer data
    glBindBuffer(GL_ARRAY_BUFFER, guiBuffer->vboGpuId);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(unsigned short) * guiBuffer->indexCount, guiBuffer->indicies);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GuiVert) * guiBuffer->vertCount, guiBuffer->verts);
}

static void Gl_RenderBg(Renderer* gl,
                         const Frustum* cam,
                         const Engine* engine,
                         const BgBuffer* buffer)
{
    Gl2Context* ctx = gl->context;
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);

    GlProg* bg = ctx->programs + kGl2ProgramBg;
    glUseProgram(bg->programId);
    
    glUniform1i(GlProg_UniformLoc(bg, kProgLocAlbedo), 0);
    glUniform1i(GlProg_UniformLoc(bg, kProgLocDepth), 1);

    glBindTexture(GL_TEXTURE_2D, engine->renderSystem.textures[TEX_VIEW_BG].gpuId);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, engine->renderSystem.textures[TEX_VIEW_BG_DEPTH].gpuId);

    glBindVertexArray(buffer->vaoGpuId);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
    
    glActiveTexture(GL_TEXTURE0);
}


static void Gl_RenderActors(Renderer* gl,
                            const Frustum* cam,
                            const Engine* engine,
                            const RenderList* renderList)
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_FALSE);
    
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 0, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
    
    glEnable(GL_BLEND);
    
    Gl2Context* ctx = gl->context;
    
    GlProg* shadowProg = ctx->programs + kGl2ProgramSkelSolid;
    glUseProgram(shadowProg->programId);
    glUniformMatrix4fv(GlProg_UniformLoc(shadowProg, kProgLocProjection), 1, GL_FALSE, Frustum_ProjMatrix(cam)->m);
    glUniformMatrix4fv(GlProg_UniformLoc(shadowProg, kProgLocView), 1, GL_FALSE, Frustum_ViewMatrix(cam)->m);

    for (int i = 0; i < renderList->skelActorCount; ++i)
    {
        const Actor* actor = engine->sceneSystem.actors + renderList->skelActors[i];
        const SkelModel* model = &actor->skelModel;
        
        glBindVertexArray(model->skin.vaoGpuId);
        
        glUniform4fv(GlProg_UniformLoc(shadowProg, kProgLocJointRotations), model->skel.jointCount, (float*)model->skel.renderJointRotations);
        glUniform3fv(GlProg_UniformLoc(shadowProg, kProgLocJointOrigins), model->skel.jointCount, (float*)model->skel.renderJointOrigins);
        
        
        float brightness[SCENE_LIGHTS_PER_VIEW] = { 0.24f, 0.1f};

        for (int j = 0; j < SCENE_LIGHTS_PER_VIEW; ++j)
        {
            const SceneLight* light = engine->sceneSystem.activeLights[j];
            
            Vec3 shadowNormal = Vec3_Create(0.0f, 0.0f, 1.0f);
            
            if (actor->pathPoly != NULL)
                shadowNormal = actor->pathPoly->plane.normal;
            
            Plane shadowPlane = Plane_Create(Vec3_Offset(actor->position, 0.0f, 0.0f, -0.05f), shadowNormal);
            
            Mat4 object;
            Mat4 shadowTransform = Mat4_CreateShadow(shadowPlane, light->position);
            Mat4_Mult(&actor->worldMatrix, &shadowTransform, &object);
            
            glUniformMatrix4fv(GlProg_UniformLoc(shadowProg, kProgLocModel), 1, GL_FALSE, object.m);
            glUniform4f(GlProg_UniformLoc(shadowProg, kProgLocColor), 0.0f, 0.0f, 0.0f, brightness[j]);

            glDrawArrays(GL_TRIANGLES, 0, model->skin.vertCount);

        }
    }

    glDisable(GL_STENCIL_TEST);
    
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
    
    GlProg* skelProg = ctx->programs + kGl2ProgramSkelLit;
    glUseProgram(skelProg->programId);
    
    glUniformMatrix4fv(GlProg_UniformLoc(skelProg, kProgLocProjection), 1, GL_FALSE, Frustum_ProjMatrix(cam)->m);
    glUniformMatrix4fv(GlProg_UniformLoc(skelProg, kProgLocView), 1, GL_FALSE, Frustum_ViewMatrix(cam)->m);
    glUniform1i(GlProg_UniformLoc(skelProg, kProgLocAlbedo), 0);
    glUniform1i(GlProg_UniformLoc(skelProg, kProgLocNormal), 1);
    glUniform1i(GlProg_UniformLoc(skelProg, kProgLocSpecular), 2);
    glUniform1i(GlProg_UniformLoc(skelProg, kProgLocGloss), 3);
    glUniform1i(GlProg_UniformLoc(skelProg, kProgLocEnvMap), 4);
    
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_CUBE_MAP, engine->renderSystem.textures[TEX_VIEW_CUBE].gpuId);

    glActiveTexture(GL_TEXTURE0);

    glUniform3f(GlProg_UniformLoc(skelProg, kProgLocCamPosition), cam->position.x, cam->position.y, cam->position.z);

    for (int i = 0; i < renderList->skelActorCount; ++i)
    {
        const Actor* actor = engine->sceneSystem.actors + renderList->skelActors[i];
        const SkelModel* model = &actor->skelModel;
        
        Vec3 p[SCENE_LIGHTS_PER_VIEW];
        
        for (int j = 0; j < SCENE_LIGHTS_PER_VIEW; ++j)
            p[j] = engine->sceneSystem.activeLights[j]->position;
        
        glUniform3fv(GlProg_UniformLoc(skelProg, kProgLocLightPositions), SCENE_LIGHTS_PER_VIEW, &p[0].x);
        
        if (model->material.albedoMap != -1)
        {
            glBindTexture(GL_TEXTURE_2D, engine->renderSystem.textures[model->material.albedoMap].gpuId);
        }
        
        if (model->material.normalMap != -1)
        {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, engine->renderSystem.textures[model->material.normalMap].gpuId);
        }
        
        if (model->material.specularMap != -1)
        {
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, engine->renderSystem.textures[model->material.specularMap].gpuId);
        }
        
        
        if (model->material.glossMap != -1)
        {
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, engine->renderSystem.textures[model->material.glossMap].gpuId);
        }
        
        
        glBindVertexArray(model->skin.vaoGpuId);
        
        glUniform4fv(GlProg_UniformLoc(skelProg, kProgLocJointRotations), model->skel.jointCount, (float*)model->skel.renderJointRotations);
        glUniform3fv(GlProg_UniformLoc(skelProg, kProgLocJointOrigins), model->skel.jointCount, (float*)model->skel.renderJointOrigins);

        glUniformMatrix4fv(GlProg_UniformLoc(skelProg, kProgLocModel), 1, GL_FALSE, actor->worldMatrix.m);

        glDrawArrays(GL_TRIANGLES, 0, model->skin.vertCount);
        
        glActiveTexture(GL_TEXTURE0);
    }
    
    GlProg* staticProg = ctx->programs + kGl2ProgramStaticLit;
    
    glUseProgram(staticProg->programId);
    glUniformMatrix4fv(GlProg_UniformLoc(staticProg, kProgLocProjection), 1, GL_FALSE, Frustum_ProjMatrix(cam)->m);
    glUniformMatrix4fv(GlProg_UniformLoc(staticProg, kProgLocView), 1, GL_FALSE, Frustum_ViewMatrix(cam)->m);
    glUniform1i(GlProg_UniformLoc(staticProg, kProgLocAlbedo), 0);
    glUniform1i(GlProg_UniformLoc(staticProg, kProgLocSpecular), 1);
    
    glUniform3f(GlProg_UniformLoc(staticProg, kProgLocCamPosition), cam->position.x, cam->position.y, cam->position.z);
    
    for (int i = 0; i < renderList->staticActorCount; ++i)
    {
        const Actor* actor = engine->sceneSystem.actors + renderList->staticActors[i];
        
        Vec3 p[SCENE_LIGHTS_PER_VIEW];
        
        for (int j = 0; j < SCENE_LIGHTS_PER_VIEW; ++j)
            p[j] = engine->sceneSystem.activeLights[j]->position;

        glUniform3fv(GlProg_UniformLoc(skelProg, kProgLocLightPositions), SCENE_LIGHTS_PER_VIEW, &p[0].x);

        const StaticModel* model = &actor->staticModel;
        
        if (model->material.albedoMap != -1)
        {
            glBindTexture(GL_TEXTURE_2D, engine->renderSystem.textures[model->material.albedoMap].gpuId);
        }

        if (model->material.specularMap != -1)
        {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, engine->renderSystem.textures[model->material.specularMap].gpuId);
        }
        
        glBindVertexArray(model->mesh.vaoGpuId);
        
        glUniformMatrix4fv(GlProg_UniformLoc(staticProg, kProgLocModel), 1, GL_FALSE, actor->worldMatrix.m);

        glDrawArrays(GL_TRIANGLES, 0, model->mesh.vertCount);
        
        glActiveTexture(GL_TEXTURE0);
    }

}

static void Gl_RenderHints(Renderer* gl,
                           const Frustum* cam,
                           const Engine* engine,
                           const HintBuffer* buffer)
{
    if (buffer->vertCount < 1)
        return;
    
    glDepthFunc(GL_ALWAYS);

    Gl2Context* ctx = gl->context;
    
    GlProg* hintProg = ctx->programs + kGl2ProgramHint;
    glUseProgram(hintProg->programId);
    
    glUniformMatrix4fv(GlProg_UniformLoc(hintProg, kProgLocView), 1, GL_FALSE, Frustum_ViewMatrix(cam)->m);
    glUniformMatrix4fv(GlProg_UniformLoc(hintProg, kProgLocProjection), 1, GL_FALSE, Frustum_ProjMatrix(cam)->m);

    glBindVertexArray(buffer->vaoGpuId);
    glDrawElements(GL_LINES, buffer->indexCount, GL_UNSIGNED_SHORT, NULL);
}


static void Gl_Render(Renderer* gl,
                       const Frustum* cam,
                       const Engine* engine,
                       const RenderList* renderList)
{
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glScissor(0, 0, engine->renderSystem.viewportWidth * engine->renderSystem.scaleFactor, engine->renderSystem.viewportHeight * engine->renderSystem.scaleFactor);

    Gl_UpdateBuffers(gl, engine, &engine->renderSystem.hintBuffer, &engine->guiSystem.buffer);

    /* "When you need to modify OpenGL ES resources, schedule those modifications at the beginning or end of a frame." */
    /* profiling shows UpdateBuffers is more effecient at the beginning than the end */
    Gl_UpdateBuffers(gl, engine, &engine->renderSystem.hintBuffer, &engine->guiSystem.buffer);
    
    Gl_RenderBg(gl, cam, engine, &engine->renderSystem.bgBuffer);
    Gl_RenderActors(gl, cam, engine, renderList);
    //Gl_RenderHints(gl, cam, engine, &engine->renderSystem.hintBuffer);
}

static void Gl_FlushLoad(Renderer* gl)
{
    glFlush();
}

int Gl2_Init(Renderer* gl)
{
    gl->init = Gl_Init;
    gl->shutdown = Gl_Shutdown;
    gl->render = Gl_Render;
    gl->uploadTexture = Gl_UploadTexture;
    gl->uploadMesh = Gl_UploadMesh;
    gl->uploadSkelSkin = Gl_UploadSkelSkin;
    gl->cleanupSkelSkin = Gl_CleanupSkelSkin;
    gl->cleanupTexture = Gl_CleanupTexture;
    gl->cleanupMesh = Gl_CleanupMesh;
    
    gl->prepareGuiBuffer = Gl_PrepareGuiBuffer;
    gl->cleanupGuiBuffer = Gl_CleanupGuiBuffer;
    gl->prepareHintBuffer = Gl_PrepareHintBuffer;
    gl->cleanupHintBuffer = Gl_CleanupHintBuffer;
    gl->prepareBgBuffer = Gl_PrepareBgBuffer;
    
    gl->context = malloc(sizeof(Gl2Context));
    
    gl->flushLoad = Gl_FlushLoad;
    return 1;
}

void Gl2_Shutdown(Renderer* renderer)
{
    if (renderer->context)
    {
        free(renderer->context);
    }
}
