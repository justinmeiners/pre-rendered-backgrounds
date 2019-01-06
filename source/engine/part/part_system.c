
#include "part_system.h"
#include <assert.h>

static const PartEffectInfo g_partInfo[] =
{
    /* partCount, frameCount */
    -1, -1,  /* none */
    15, 10,  /* blood */
    20, 80,  /* death */
    25, 80,  /* magic */
    20, 50,  /* heal */
    40, 90,  /* mind control */
    30, 120, /* spawn */
    40, 90,  /* teleport */
};

int PartEmitter_Init(PartEmitter* emitter, int index, PartEffect effect)
{
    if (!emitter)
        return 0;
    
    emitter->position = Vec3_Zero;
    emitter->scale = Vec3_Create(1.0f, 1.0f, 1.0f);
    
    emitter->partCount = 0;
    emitter->frame = 0;
    emitter->loops = 0;
    
    emitter->index = index;
    
    emitter->effect = effect;
    emitter->effectInfo = g_partInfo + effect;
    
    emitter->boundsRadius = 25.0f;
    
    return 1;
}

float PartEmitter_CalcTime(const PartEmitter* emitter)
{
    if (!emitter->effect)
        return 0.0f;
    
    assert(emitter->effectInfo->frameCount != 0);
    return emitter->frame / (float)emitter->effectInfo->frameCount;
}

int PartSystem_Init(PartSystem* system)
{
    if (system)
    {
        for (int i = 0; i < PART_SYSTEM_EMITTERS_MAX; ++i)
        {
            PartEmitter_Init(system->emitters + i, i, kPartEffectNone);
        }

        return 1;
    }
   
    return 0;
}

void PartSystem_Tick(PartSystem* system)
{
    for (int i = 0; i < PART_SYSTEM_EMITTERS_MAX; ++i)
    {
        PartEmitter* emitter = system->emitters + i;
        
        if (emitter->effect != kPartEffectNone && emitter->playing)
        {
            assert(emitter->effectInfo->frameCount != 0);
            
            if (emitter->frame == 0)
            {
                emitter->partCount = emitter->effectInfo->initialPartCount;
            }
            else if (emitter->frame == emitter->effectInfo->frameCount - 1)
            {
                if (!emitter->loops)
                {
                    emitter->playing = 0;
                    system->emitters[i].effect = kPartEffectNone;
                    continue;
                }
            }
            
            emitter->frame = (emitter->frame + 1) % emitter->effectInfo->frameCount;
        }
    }
}

PartEmitter* PartSystem_PrepareEffect(PartSystem* system, const PartEffect effect)
{
    assert(system);
    
    PartEmitter* source = NULL;
    
    int i;
    for (i = 0; i < PART_SYSTEM_EMITTERS_MAX; ++i)
    {
        if (system->emitters[i].effect == kPartEffectNone)
        {
            source = system->emitters + i;
            break;
        }
    }
    
    if (!source)
    {
        return NULL;
    }
    
    PartEmitter_Init(source, i, effect);
    source->playing = 1;
    
    return source;
}

void PartSystem_Play(PartSystem* system, PartEmitter* source)
{
    source->playing = 1;
}

void PartSystem_Stop(PartSystem* system, PartEmitter* source)
{
    source->playing = 0;
}

PartEmitter* PartSystem_EmitEffect(PartSystem* system,
                                   PartEffect effect,
                                   Vec3 position,
                                   Quat rotation,
                                   int loop)
{
    PartEmitter* source = PartSystem_PrepareEffect(system, effect);
    
    if (source)
    {
        source->loops = loop;
        source->position = position;
    }
    
    return source;
}
