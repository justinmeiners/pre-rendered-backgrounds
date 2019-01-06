# [READ ARTICLE HERE](https://justinmeiners.github.io/pre-rendered-backgrounds)

# An Adventure in Pre-Rendered Backgrounds.

Created by programmer [Justin Meiners](https://github.com/justinmeiners) and 3D artist [Hunter Rassmussen](https://github.com/).

This repository contains the entire source code for our game prototype project, as well as an article summarizing our work. 
It also contains the source code for a game engine written in C focused on mobile devices.

## Building on macOS

1. Install [SDL 2](https://www.libsdl.org)

Hombrew makes this easy

``` 
brew install sdl2
```

2. Build and run in Xcode

## Building on other Platforms

This project was written for macOS although it should be reasonably portable to other systems. The bulk of the code is portable C, or C which can be trivially adjusted to build on other platforms.

It also depends on:

- [SDL 2](https://www.libsdl.org). Just used for windowing and input. This can be replaced with any other method for creating an OpenGL context.

- [OpenGL 3.2](https://en.wikipedia.org/wiki/OpenGL#OpenGL_3.2)

- [Core Audio](https://developer.apple.com/library/archive/documentation/MusicAudio/Conceptual/CoreAudioOverview/WhatisCoreAudio/WhatisCoreAudio.html)

The engine uses an abstraction layer between [OpenGL renderer](source/platform/gl_3/gl_3.h) and [CoreAudio](source/platform/core_audio/snd_driver_core_audio.h). These modules are  independent of the rest of the engine and could be easily replaced with another implementation. For example, the OpenGL renderer could be replaced with one for Metal or DirectX.

### iOS

Another game I wrote uses this engine on iOS. To port to iOS the SDL windowing need to be replaced with a standard iOS OpenGL view. The shaders needs to be changed from 3.2 grammer to ES 2.0.

## Structure

- `data/` game data such as models, textures, etc.
- `source/engine/` portable game and engine code.
- `source/platform/` API or platform specific modules.
- `support/` project files for Xcode or your IDE.
- `tools/` asset creation tools such as scripts for Blender.

## Game Data

To run the game you will need to acquire the data. You can download this from the [Releases](https://github.com/justinmeiners/pre-rendered-backgrounds) page.

# License

The code is licensed under [GPL](LICENSE).

