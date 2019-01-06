

#ifndef PART_SYSTEM_H
#define PART_SYSTEM_H

#include "vec_math.h"

/*
 Particle system is designed to be entirely stateless to:
 - allow parallel processing through a GPU shader
 - reduce memory consumption
 - simplifiy effect creation processes
 */


/*
 part nodes provide external data to particle system.
 This is to allow effects such as flamethrowers, charging beams,
 etc which require a target to flow to.
 */

typedef struct
{
    int hidden;
    Vec3 position;
    Vec4 color;
} PartState;


typedef struct
{
    int initialPartCount;
    int frameCount;
    
} PartEffectInfo;

typedef enum
{
    kPartEffectNone = 0,
    kPartEffectBlood,
    kPartEffectSmoke,
    kPartEffectMagic,
    kPartEffectHeal,
    kPartEffectMindControl,
    kPartEffectSpawn,
    kPartEffectTeleport,
} PartEffect;


struct PartEmitter;

#define PART_EMITTER_NODES_MAX 4

typedef struct
{
    int index;
    
    Vec3 position;
    Vec3 scale;
    float boundsRadius;
    
    PartEffect effect;
    const PartEffectInfo* effectInfo;
    
    Vec3 controlPoints[PART_EMITTER_NODES_MAX];
    
    int partCount;
    int frame;
    
    int loops;
    int playing;
    
} PartEmitter;

extern int PartEmitter_Init(PartEmitter* emitter, int index, PartEffect effect);

extern float PartEmitter_CalcTime(const PartEmitter* emitter);



#define PART_SYSTEM_EMITTERS_MAX 16

typedef struct
{
    
    PartEmitter emitters[PART_SYSTEM_EMITTERS_MAX];
} PartSystem;

extern int PartSystem_Init(PartSystem* system);

extern void PartSystem_Tick(PartSystem* system);

extern PartEmitter* PartSystem_PrepareEffect(PartSystem* system, const PartEffect effect);

extern void PartSystem_Play(PartSystem* system, PartEmitter* source);
extern void PartSystem_Stop(PartSystem* system, PartEmitter* source);

extern PartEmitter* PartSystem_EmitEffect(PartSystem* system,
                                          PartEffect effect,
                                          Vec3 position,
                                          Quat rotation,
                                          int loop);


#endif /* part_system_h */
