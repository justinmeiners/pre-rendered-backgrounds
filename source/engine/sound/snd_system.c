#include "snd_system.h"
#include <assert.h>
#include "utils.h"

static void SndEmitter_Init(SndEmitter* source)
{
    source->snd = NULL;
    source->volume = 1.0f;
    source->playing = 0;
    source->positionEnabled = 0;
    source->sampleIndex = 0;
    source->looping = 0;
    source->group = kSndGroupEffects;
}

static void SndEmitter_Restart(SndEmitter* source)
{
    source->sampleIndex = 0;
    source->subCounter = 0;
}

static int SndDriver_Prepare(SndDriver* driver)
{
    assert(!driver->running);
    
    if (driver->prime && driver->prime(driver))
    {
        return 1;
    }
    
    return 0;
}

static int SndDriver_Start(SndDriver* driver)
{
    if (driver->start && driver->start(driver))
    {
        driver->running = 1;
        return 1;
    }
    
    return 0;
}

static void SndDriver_Stop(SndDriver* driver)
{
    if (driver->running)
    {
        driver->running = 0;
        driver->stop(driver);
    }
}


static void SndDriver_Callback(void* delegate, struct SndDriver* driver, float* leftOut, float* rightOut)
{
    SndSystem* env = delegate;
    
    if (!env)
    {
        return;
    }
    
    *leftOut = 0.0f;
    *rightOut = 0.0f;
    
    for (int i = 0; i < SND_SYSTEM_MAX_SOURCES; ++i)
    {
        float sourceLeft = 0.0f;
        float sourceRight = 0.0f;
        
        SndEmitter* source = env->sources + i;
        
        if (!source->snd || !source->playing)
            continue;
        
        if (source->positionEnabled)
        {
            float val = Snd_UnpackSample(source->snd, source->sampleIndex, 0);

            Vec3 diff = Vec3_Norm(Vec3_Sub(source->position, env->listener.position));
            
            float dot = Vec3_Dot(diff, env->listener.right);
            
            if (dot > 0.0f)
            {
                sourceLeft = (val * dot);
            }
            else
            {
                sourceRight = (val * dot);
            }
        }
        else
        {
            sourceLeft = Snd_UnpackSample(source->snd, source->sampleIndex, 0);
            sourceRight = Snd_UnpackSample(source->snd, source->sampleIndex, 1);
        }
        
        source->subCounter++;
        
        if (source->subCounter >= (driver->sampleRate / source->snd->sampleRate))
        {
            source->subCounter = 0;
            source->sampleIndex ++;
        }
        
        if (source->sampleIndex >= source->snd->sampleCount)
        {
            source->sampleIndex = 0;
            
            if (!source->looping)
            {
                source->playing = 0;
                source->snd = NULL;
            }
        }
        
        SndGroupInfo* group = &env->groups[source->group];
        
        *leftOut += sourceLeft * source->volume * group->volume;
        *rightOut += sourceRight * source->volume * group->volume;
    }
    
    *leftOut = *leftOut * env->masterVolume;
    *rightOut = *rightOut * env->masterVolume;
}


int SndSystem_Init(SndSystem* system, SndDriver* driver)
{
    if (!system)
        return 0;
    
    if (driver)
    {
        system->driver = driver;
        system->driver->delegate = system;
        system->driver->callback = SndDriver_Callback;
    }
    else
    {
        system->driver = NULL;
    }
    
    system->masterVolume = 0.25f;
    
    for (int i = 0; i < kSndGroupCount; ++i)
    {
        system->groups[i].volume = 1.0f;
    }
    
    for (int i = 0; i < SND_SYSTEM_MAX_SOURCES; ++i)
    {
        SndEmitter_Init(system->sources + i);
    }
    
    system->ambient = NULL;
    
    if (driver)
    {
        SndDriver_Prepare(system->driver);
        SndDriver_Start(system->driver);
    }
    
    return 1;
}

void SndSystem_Shutdown(SndSystem* env)
{
    SndDriver_Stop(env->driver);
}

SndEmitter* SndSystem_PrepareSound(SndSystem* env, const Snd* sound)
{
    if (!env || !sound)
        return NULL;

    SndEmitter* source = NULL;
    
    for (int i = 0; i < SND_SYSTEM_MAX_SOURCES; i++)
    {
        if (env->sources[i].snd == NULL)
        {
            source = env->sources + i;
            break;
        }
    }
    
    if (!source)
        return NULL;
    
    source->snd = sound;
    SndEmitter_Restart(source);
    
    return source;
}


void SndSystem_LoadSnd(SndSystem* system, int soundIndex, const char* path)
{
    assert(soundIndex < SND_SYSTEM_MAX_SNDS);
    Snd* sound = system->sounds + soundIndex;
    
    if (path == NULL)
    {
        Snd_Shutdown(sound);
        return;
    }
    
    char fullPath[MAX_OS_PATH];
    Filepath_Append(fullPath, Filepath_DataPath(), path);
    
    Snd_FromPath(sound, fullPath);
}

int SndSystem_PlaySound(SndSystem* system, int sound)
{
    SndEmitter* source = SndSystem_PrepareSound(system, system->sounds + sound);
    
    if (!source)
        return 0;
    
    source->playing = 1;
    return 1;
}

void SndSystem_SetAmbient(SndSystem* system, int sound)
{
    if (system->ambient)
    {
        system->ambient->playing = 0;
        system->ambient->looping = 0;
        system->ambient->snd = NULL;
    }
    
    system->ambient = SndSystem_PrepareSound(system, system->sounds + sound);
    system->ambient->group = kSndGroupAmbient;
    system->ambient->looping = 1;
    system->ambient->playing = 1;
}

