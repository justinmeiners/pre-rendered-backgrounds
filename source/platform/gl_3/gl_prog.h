
#ifndef GL_PROG_H
#define GL_PROG_H

#include "gl_compat.h"

#define GL_UNIFORM_NAME_MAX 64


typedef enum
{
    kGlAttribVertex = 0,
    kGlAttribNormal,
    kGlAttribTangent,
    kGlAttribUv0,
    kGlAttribUv1,
    
    /* parts */
    kGlAttribIndex,
    
    /* Skel */
    kGlAttribWeightJoints,
    kGlAttribWeight0,
    kGlAttribWeight1,
    kGlAttribWeight2,
    
    /* GUI */
    kGlAttribAtlasId,
    kGlAttribColor,

    
} GlAttrib;


typedef struct
{
    char name[GL_UNIFORM_NAME_MAX];
    GLint location;
    GLint type;
} GlUniformInfo;

typedef enum
{
    kGlShaderTypeVertex = 0,
    kGlShaderTypeFragment,
    kGlShaderTypeCount
} GlShaderType;

typedef struct
{
    GLuint shaderID;
    GlShaderType type;
} GlShader;

/*
 "If your shaders must use branches, follow these recommendations:
 
 Best performance: Branch on a constant known when the shader is compiled.
 Acceptable: Branch on a uniform variable.
 Potentially slow: Branch on a value computed inside the shader." - Apple
*/

#define PROG_UNIFORMS_MAX 48

typedef struct
{
    int linked;

    GLuint programId;
    GlShader shaders[kGlShaderTypeCount];
    int attachedFlag[kGlShaderTypeCount];
    
    int uniformCount;
    GlUniformInfo uniforms[PROG_UNIFORMS_MAX];
    
    /*
     I started implementing a hashmap here to avoid linear lookup but it turned out to be ineffective.
     With a 1024 bucket's containing array indicies I was getting collisions (7-8 uniforms), so fixing the collisions would require
     linked list chaining. Chaining would require dynamic allocation or a stack pool of some kind.
     
     A full string search is currently < 16 string compares which is a tiny number of iterations. The hash function itself probably uses more iterations in its calculation.
    
     This allows uniform locations to be mapped to predefined incidies for constant time lookup */
    int locTable[PROG_UNIFORMS_MAX];
    
} GlProg;

extern int GlProg_Init(GlProg* program);
extern int GlProg_InitWithPaths(GlProg* program,
                                const char* vertexPath,
                                const char* fragmentPath);

extern void GlProg_Shutdown(GlProg* program);

extern void GlProg_Attach(GlProg* program, GlShaderType type);

extern int GlProg_CompilePath(GlProg* program,
                                 GlShaderType type,
                                 const char* path,
                                 int debug);

extern int GlProg_Compile(GlProg* program,
                          GlShaderType type,
                          const char* source,
                          int debug);

extern int GlProg_Link(GlProg* program, int debug);

extern GlUniformInfo* GlProg_FindUniform(GlProg* program, const char* name);
extern int GlProg_FindUniformLoc(GlProg* program, const char* name);

/* returns the uniform location at the table index */
extern int GlProg_UniformLoc(GlProg* program, int tableIndex);

/* stores a uniform location in the loc table at tableIndex */
extern void GlProg_MapUniformLoc(GlProg* program, const char* name, int tableIndex);

/* Must be called before link wrapper for glBindAttrib. */
extern void GlProg_BindAttrib(GlProg* program, GlAttrib attrib, const char* name);

#endif
