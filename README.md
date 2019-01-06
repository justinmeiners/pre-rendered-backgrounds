# An Adventure in Pre-Rendered Backgrounds.

# [READ HERE](https://justinmeiners.github.io/pre-rendered-backgrounds)

Created by programmer [Justin Meiners](https://justinmeiners.github.io) and 3D artist [Hunter Rassmussen](https://github.com/HunterRasmussen).

This repository contains:
- the source code for our pre-rendered backgrounds game prototype.
- a summary of our project.
- the source code for a game engine written in C focused on mobile devices.

## Building on macOS

1. Install [SDL 2](https://www.libsdl.org)

   Hombrew makes this easy:
   ```
   brew install sdl2
   ```

2. Build and run in Xcode

## Building on other Platforms

This project was written for macOS, although it should be reasonably portable to other systems. The bulk of the code is portable C, or C which can be trivially adjusted to build on other platforms.

It also depends on:

- [SDL 2](https://www.libsdl.org). Just used for windowing and input. This can be replaced with any other method for creating an OpenGL context.

- [OpenGL 3.2](https://en.wikipedia.org/wiki/OpenGL#OpenGL_3.2)

- [Core Audio](https://developer.apple.com/library/archive/documentation/MusicAudio/Conceptual/CoreAudioOverview/WhatisCoreAudio/WhatisCoreAudio.html)

The engine uses an abstraction layer between [OpenGL renderer](source/platform/gl_3/) and [CoreAudio](source/platform/core_audio/). These modules are  independent of the rest of the engine and could be easily replaced with another implementation. For example, the OpenGL renderer could be replaced with one for Metal or DirectX.

### iOS

Another game I wrote uses this engine on iOS. To port to iOS the SDL windowing need to be replaced with a standard iOS OpenGL view. The shaders need to be changed from 3.2 GLSL grammer to the ES 2.0 version.

## Structure

- `data/` game data such as models, textures, etc.
- `docs/` the summary of our project.
- `source/engine/` portable game and engine code.
- `source/platform/` API or platform specific modules.
- `support/` project files for Xcode or your IDE.
- `tools/` asset creation tools such as scripts for Blender.

## Game Data

To run the game, you will need to acquire the data. You can download this from the [releases](https://github.com/justinmeiners/pre-rendered-backgrounds) page.

# License

The code is licensed under [GPL](LICENSE).

