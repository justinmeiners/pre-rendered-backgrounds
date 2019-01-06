#ifndef ASSET_MANIFEST_H
#define ASSET_MANIFEST_H

#define ASSET_NAME_MAX 128

typedef struct
{
    int identifier;
    const char* name;
    const char* path;
} AssetEntry;

enum
{
    TEX_ASTRONAUT_ALBEDO,
    TEX_ASTRONAUT_GLOSS,
    TEX_ASTRONAUT_NORMAL,
    TEX_ASTRONAUT_SPECULAR,
    TEX_VIEW_BG,
    TEX_VIEW_BG_DEPTH,
    TEX_VIEW_CUBE,
    TEX_WRENCH_ALBEDO,
    TEX_WRENCH_SPECULAR,
};

enum
{
    SKEL_ASTRONAUT,
};

enum
{
    ANIM_ASTRONAUT_IDLE,
    ANIM_ASTRONAUT_PRESS,
    ANIM_ASTRONAUT_WALK,
};

enum
{
    SOUND_BEEP,
    SOUND_GENERATOR,
    SOUND_TUNNEL,
};

enum
{
    SCRIPT_GLOBAL,
    SCRIPT_LEVEL,
};

enum
{
    MODEL_WRENCH,
};

enum { Asset_textureCount=9 };

static const AssetEntry Asset_textureManifest[] = {
    TEX_ASTRONAUT_ALBEDO, "TEX_ASTRONAUT_ALBEDO", "characters/astronaut_albedo.png",
    TEX_ASTRONAUT_GLOSS, "TEX_ASTRONAUT_GLOSS", "characters/astronaut_gloss.png",
    TEX_ASTRONAUT_NORMAL, "TEX_ASTRONAUT_NORMAL", "characters/astronaut_normal.png",
    TEX_ASTRONAUT_SPECULAR, "TEX_ASTRONAUT_SPECULAR", "characters/astronaut_specular.png",
    TEX_VIEW_BG, "TEX_VIEW_BG", NULL,
    TEX_VIEW_BG_DEPTH, "TEX_VIEW_BG_DEPTH", NULL,
    TEX_VIEW_CUBE, "TEX_VIEW_CUBE", NULL,
    TEX_WRENCH_ALBEDO, "TEX_WRENCH_ALBEDO", "props/wrench_albedo.png",
    TEX_WRENCH_SPECULAR, "TEX_WRENCH_SPECULAR", "props/wrench_specular.png",
};

enum { Asset_skelModelCount=1 };

static const AssetEntry Asset_skelModelManifest[] = {
    SKEL_ASTRONAUT, "SKEL_ASTRONAUT", "characters/astronaut.skmesh",
};

enum { Asset_skelAnimCount=3 };

static const AssetEntry Asset_skelAnimManifest[] = {
    ANIM_ASTRONAUT_IDLE, "ANIM_ASTRONAUT_IDLE", "characters/astronaut_idle.skanim",
    ANIM_ASTRONAUT_PRESS, "ANIM_ASTRONAUT_PRESS", "characters/astronaut_press.skanim",
    ANIM_ASTRONAUT_WALK, "ANIM_ASTRONAUT_WALK", "characters/astronaut_walk.skanim",
};

enum { Asset_soundCount=3 };

static const AssetEntry Asset_soundManifest[] = {
    SOUND_BEEP, "SOUND_BEEP", "sounds/beep.wav",
    SOUND_GENERATOR, "SOUND_GENERATOR", "sounds/generator.wav",
    SOUND_TUNNEL, "SOUND_TUNNEL", "sounds/tunnel.wav",
};

enum { Asset_scriptCount=2 };

static const AssetEntry Asset_scriptManifest[] = {
    SCRIPT_GLOBAL, "SCRIPT_GLOBAL", NULL,
    SCRIPT_LEVEL, "SCRIPT_LEVEL", NULL,
};

enum { Asset_staticModelCount=1 };

static const AssetEntry Asset_staticModelManifest[] = {
    MODEL_WRENCH, "MODEL_WRENCH", "props/wrench.mesh",
};

#endif
