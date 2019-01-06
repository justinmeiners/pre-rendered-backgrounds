
#include "gl_prog.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>


static const char g_fragShaderPrefix[] = "\
#version 330\n\
precision mediump float; \n \
";

static const char g_vertShaderPrefix[] = "\
#version 330\n \
";


static inline int GLUniformCompare(const void* a, const void* b)
{
    const GlUniformInfo* ua = a;
    const GlUniformInfo* ub = b;
    return strncmp(ua->name, ub->name, GL_UNIFORM_NAME_MAX);
}


int GlProg_Init(GlProg* program)
{
    if (!program)
        return 0;
    
    program->programId = glCreateProgram();
    program->linked = 0;
    program->uniformCount = 0;
    
    for (int i = 0; i < PROG_UNIFORMS_MAX; ++i)
    {
        program->locTable[0] = -1;
    }
    
    return 1;
}

int GlProg_InitWithPaths(GlProg* program,
                         const char* vertexPath,
                         const char* fragmentPath)
{
    int status = GlProg_Init(program);
    if (!status)
        return 0;
    
    GlProg_CompilePath(program, kGlShaderTypeVertex, vertexPath, 1);
    GlProg_CompilePath(program, kGlShaderTypeFragment, fragmentPath, 1);
    
    GlProg_Attach(program, kGlShaderTypeVertex);
    GlProg_Attach(program, kGlShaderTypeFragment);
    
    return status;
}

void GlProg_Shutdown(GlProg* program)
{
    for (int i = 0; i < kGlShaderTypeCount; ++i)
    {
        if (program->attachedFlag[i])
        {
            glDeleteShader(program->shaders[i].shaderID);
        }
    }
    
    glDeleteProgram(program->programId);
}


void GlProg_Attach(GlProg* program, GlShaderType type)
{
    glAttachShader(program->programId, program->shaders[type].shaderID);
    program->attachedFlag[type] = 1;
}

int GlProg_CompilePath(GlProg* program,
                              GlShaderType type,
                              const char* path,
                              int debug)
{
    FILE* file = fopen(path, "r");
    
    if (!file)
    {
        if (debug)
        {
            printf("could not load path: %s\n", path);
        }
        return 0;
    }
    
    fseek(file, 0, SEEK_END);
    size_t length = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* buffer = malloc(length + 1);
    if (!buffer)
    {
        fclose(file);
        return 0;
    }
    
    fread(buffer, length, 1, file);
    fclose(file);
    
    buffer[length] = '\0';
    
    int retValue = GlProg_Compile(program, type, buffer, debug);
    free(buffer);
    return retValue;
}

int GlProg_Compile(GlProg* program,
                   GlShaderType type,
                   const char* source,
                   int debug)
{
    GlShader* shader = program->shaders + type;
    
    shader->type = type;
        
    GLenum glShaderType = 0;
        
    switch (shader->type)
    {
        case kGlShaderTypeVertex:
            glShaderType = GL_VERTEX_SHADER;
            break;
        case kGlShaderTypeFragment:
            glShaderType = GL_FRAGMENT_SHADER;
            break;
        default:
            break;
    }
    
    shader->shaderID = glCreateShader(glShaderType);
    
    if (!shader->shaderID)
    {
        return 0;
    }
    
    if (type == kGlShaderTypeFragment)
    {
        const char* sources[] = {g_fragShaderPrefix, source};
        glShaderSource(shader->shaderID, 2, sources, NULL);
    }
    else
    {
        const char* sources[] = {g_vertShaderPrefix, source};
        glShaderSource(shader->shaderID, 2, sources, NULL);
    }
    
    glCompileShader(shader->shaderID);
    
    if (debug)
    {
        GLint logLength;
        glGetShaderiv(shader->shaderID, GL_INFO_LOG_LENGTH, &logLength);
        
        if (logLength > 0)
        {
            GLchar *log = (GLchar *)malloc(logLength);
            glGetShaderInfoLog(shader->shaderID, logLength, &logLength, log);
            
            const char* kind = "unknown";
            
            switch (shader->type)
            {
                case kGlShaderTypeVertex:
                    kind = "vertex";
                    break;
                case kGlShaderTypeFragment:
                    kind = "fragment";
                    break;
                default:
                    break;
            }
            
            printf("%s - compiled:\n%s\n", kind, log);
            free(log);
        }
    }
    
    GLint status;
    glGetShaderiv(shader->shaderID, GL_COMPILE_STATUS, &status);
    
    if (status == 0)
    {
        glDeleteShader(shader->shaderID);
        return 0;
    }
    
    return 1;
}

int GlProg_Link(GlProg* program, int debug)
{
    GLint status;
    
    glLinkProgram(program->programId);
    
    if (debug)
    {
        GLint logLength;
        glGetProgramiv(program->programId, GL_INFO_LOG_LENGTH, &logLength);

        if (logLength > 0)
        {
            GLchar *log = (GLchar*)malloc(logLength);
            glGetProgramInfoLog(program->programId, logLength, &logLength, log);
            
            printf("program link log:\n%s\n", log);            
            free(log);
        }
    }

    glGetProgramiv(program->programId, GL_LINK_STATUS, &status);
	
    if (status == 0)
    {
        program->linked = 0;
        return 0;
    }
    
    GLint maxUniformLength;
    GLint uniformCount;
    
    glGetProgramiv(program->programId, GL_ACTIVE_UNIFORMS, &uniformCount);
    glGetProgramiv(program->programId, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxUniformLength);
    
    program->uniformCount = uniformCount;
    assert(uniformCount < PROG_UNIFORMS_MAX);
    
    for (int i = 0; i < uniformCount; ++i)
    {
        GlUniformInfo* uniform = program->uniforms + i;
        
        GLint size;
        GLenum type;
        glGetActiveUniform(program->programId, i, maxUniformLength, NULL, &size, &type, uniform->name);
      
        uniform->location = glGetUniformLocation(program->programId, uniform->name);
        uniform->type = type;
    }
    
    // sort uniforms for binary search
    qsort(program->uniforms, program->uniformCount, sizeof(GlUniformInfo), GLUniformCompare);
    
	program->linked = 1;
    return 1;
}


GlUniformInfo* GlProg_FindUniform(GlProg* program, const char* name)
{
    GlUniformInfo key;
    strncpy(key.name, name, GL_UNIFORM_NAME_MAX);
    return bsearch(&key, program->uniforms, program->uniformCount, sizeof(GlUniformInfo), GLUniformCompare);
}

int GlProg_FindUniformLoc(GlProg* program, const char* name)
{
    GlUniformInfo* info = GlProg_FindUniform(program, name);
    if (!info) return 0;
    return info->location;
}

int GlProg_UniformLoc(GlProg* program, int tableIndex)
{
    return program->locTable[tableIndex];
}

void GlProg_MapUniformLoc(GlProg* program, const char* name, int tableIndex)
{
    assert(tableIndex < PROG_UNIFORMS_MAX);
    GlUniformInfo* info = GlProg_FindUniform(program, name);
    
    if (info)
        program->locTable[tableIndex] = info->location;
}

void GlProg_BindAttrib(GlProg* program, GlAttrib attrib, const char* name)
{
    glBindAttribLocation(program->programId, attrib, name);
}

