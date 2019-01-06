
#ifndef GL_3_H
#define GL_3_H

#include <stdio.h>
#include "engine.h"


/* 

 iPhone/iPad
 Mac
 Linux
 Windows 7
 
 
https://developer.apple.com/opengl/capabilities/
 
 COMPATIBILTY
 ---------------------
 OpenGL ES 2.0 - (PowerVR SGX. iPhone 3Gs and newer)
    - PVRTC textures https://imgtec.com/blog/pvrtc-the-most-efficient-texture-compression-standard-for-the-mobile-graphics-world/
    - ES GLSL 100 shaders
 
 OpenGL 3.2 - (All Mac 10.7 and newer)
    - DXT textures
    - GLSL 150 Shaders
 
 ARB_ES2_compatibility - (All Mac 10.7 (Lion) and newer)
 
 
  */


/*
 ARB_ES2_compatibility
 https://www.opengl.org/registry/specs/ARB/ES2_compatibility.txt
 
 "Shaders that do not include a #version directive will be
 treated as targeting version 1.10. Shaders that specify #version 100 will
 be treated as targeting version 1.00 of the OpenGL ES Shading Language,
 which is a strict subset of version 1.50."
 
 
 I have confirmed that with ES2 compatability an OpenGL 3.2 > context can correctly load
 ES 2.0 shaders. All Apple cards support ES2 extension, I assume this is for iPhone simulator.
 
 
 
 */


/*
 I am concerned about compatility because linux is important to me,
 but there is also no sense on basing the game of such a limited form of the technology.
 Shaders are essential, however I would like to get further to the game before spending too much
 time tweaking shaders and graphical effects.
 */


/* OpenGL 2.1 - According to the internet, the most compatible OpenGL version. */


extern int Gl2_Init(Renderer* renderer);
extern void Gl2_Shutdown(Renderer* renderer);

#endif
