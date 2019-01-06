

#include <stdio.h>
#include <OpenGL/gl3.h>

#include "SDL2/SDL.h"
#include "gl_3.h"
#include "snd_driver_core_audio.h"
#include "utils.h"

#define BUILD_DEBUG 1

int g_windowWidth = 1024;
int g_windowHeight = 768;

int main(int argc, const char * argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO) == -1)
    {
        printf("failed to init SDL\n");
        return 1;
    }
    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 1);

    if (!BUILD_DEBUG)
    {
        SDL_ShowCursor(SDL_DISABLE);
    }
    
    SDL_Window* window = NULL;
    SDL_GLContext* context = NULL;
    
    window = SDL_CreateWindow("Master",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              g_windowWidth,
                              g_windowHeight,
                              SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    /* Create our opengl context and attach it to our window */
    context = SDL_GL_CreateContext(window);
    
    if (!window || !context)
    {
        printf("failed to create window and context\n");
        return 1;
    }
    
    SDL_GL_SetSwapInterval(1);
    
    glViewport(0, 0, g_windowWidth, g_windowHeight);

    Filepath_SetDataPath("data/");
    
    
    Renderer gl;
    Gl2_Init(&gl);
    
    SndDriver* driver = SndDriver_CoreAudio_Create(44100);
    
    EngineSettings engineSettings;
    engineSettings.inputConfig = kInputConfigMouseKeyboard;
    engineSettings.guiWidth = 1024;
    engineSettings.guiHeight = 768;
    engineSettings.renderWidth = g_windowWidth;
    engineSettings.renderHeight = g_windowHeight;
    engineSettings.renderScaleFactor = 1.0f;
    
    if (!Engine_Init(&g_engine, &gl, driver,engineSettings))
    {
        return 2;
    }

    InputState inputState;
    InputState_Init(&inputState);

    int quit = 0;

    while (!quit)
    {
        int x, y;
        SDL_GetMouseState(&x, &y);
        
        inputState.mouseCursor.x = (x / (float)g_engine.renderSystem.viewportWidth) * 2.0f - 1.0f;
        inputState.mouseCursor.y = -((y / (float)g_engine.renderSystem.viewportHeight) * 2.0f - 1.0f);
        
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            switch (e.type)
            {
                case SDL_QUIT:
                    quit = 1;
                    break;
                case SDL_KEYUP:
                {
                    if (e.key.keysym.sym == SDLK_q)
                    {
                        quit = 1;
                    }
                }
                case SDL_MOUSEBUTTONDOWN:
                    if (e.button.button == SDL_BUTTON_LEFT) {inputState.mouseButtons[kMouseButtonLeft].down = 1; }
                    if (e.button.button == SDL_BUTTON_RIGHT) {inputState.mouseButtons[kMouseButtonRight].down = 1; }
                    break;
                case SDL_MOUSEBUTTONUP:
                    if (e.button.button == SDL_BUTTON_LEFT) {inputState.mouseButtons[kMouseButtonLeft].down = 0; }
                    if (e.button.button == SDL_BUTTON_RIGHT) {inputState.mouseButtons[kMouseButtonRight].down = 0; }
                    break;
            }
        }
        
        Engine_Update(&g_engine, &inputState);
        Engine_Render(&g_engine);
        SDL_GL_SwapWindow(window);
    }
    
    SDL_GL_DeleteContext(context);
    
    return 0;
}

