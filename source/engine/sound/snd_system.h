
#ifndef SND_SYSTEM_H
#define SND_SYSTEM_H

#include "vec_math.h"
#include "snd.h"
#include "snd_driver.h"

typedef struct
{
    float volume;
    
} SndGroupInfo;

typedef enum
{
    kSndGroupEffects = 0,
    kSndGroupAmbient,
    kSndGroupCount,
} SndGroup;

/* some point that is emitting a sound into the world eg: a speaker */
typedef struct
{
    /* voice, music, effects, etc */
    SndGroup group;
    const Snd* snd;
    int sampleIndex;
    int subCounter;
    
    int playing;
    int looping;
    float volume;
    
    int positionEnabled;
    Vec3 position;
    
} SndEmitter;

typedef struct
{
    Vec3 position;
    Vec3 right;
    Vec3 forward;
    Vec3 orientation;
    
} SndListener;

/* maximum amount of simultaneously sounds */
#define SND_SYSTEM_MAX_SOURCES 16
#define SND_SYSTEM_MAX_SNDS 64

typedef struct
{
    SndListener listener;
    SndGroupInfo groups[kSndGroupCount];
    SndEmitter sources[SND_SYSTEM_MAX_SOURCES];
    SndDriver* driver;
    float masterVolume;
    
    Snd sounds[SND_SYSTEM_MAX_SNDS];

    SndEmitter* ambient;
} SndSystem;


extern int SndSystem_Init(SndSystem* system, SndDriver* driver);
extern void SndSystem_Shutdown(SndSystem* env);

extern SndEmitter* SndSystem_PrepareSound(SndSystem* env, const Snd* sound);

extern void SndSystem_LoadSnd(SndSystem* system, int sound, const char* path);

extern int SndSystem_PlaySound(SndSystem* system, int sound);
extern void SndSystem_SetAmbient(SndSystem* system, int sound);

#endif
