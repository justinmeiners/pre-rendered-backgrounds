
#ifndef GL_COMPAT_H
#define GL_COMPAT_H

#if __APPLE__

#include "TargetConditionals.h"
    #if TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE
        #define OPENGL_ES_2 1
    #else
        #define OPENGL_ES_2 0
    #endif
#endif


#if OPENGL_ES_2

#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

#define glBindVertexArray glBindVertexArrayOES
#define glGenVertexArrays glGenVertexArraysOES

#define glDeleteVertexArrays glDeleteVertexArraysOES

#define GL_TEXTURE_MAX_LEVEL GL_TEXTURE_MAX_LEVEL_APPLE 

#define GL_RGB8 GL_RGB
#define GL_RGBA8 GL_RGBA

#else

#include <OpenGL/gl3.h>
#include <OpenGL/gl3ext.h>

#endif

#endif 
