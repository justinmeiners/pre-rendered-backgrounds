
#ifndef SKEL_ANIM_H
#define SKEL_ANIM_H

#include "skel.h"
#include "vec_math.h"


#define SKEL_ANIM_MARKER_NAME_MAX 64

typedef struct
{
    char name[SKEL_ANIM_MARKER_NAME_MAX];
    short frame;
} SkelAnimMarker;

typedef struct
{
    Quat rotation;
} SkelAnimJoint;


typedef struct
{
    short markerIndex;
    
    /* 
     this frames offest from the origin 
     - useful for determining character movement from animation
     */
    
    Vec3 rootOffset;
} SkelAnimFrame;

/*
 SkelAnim stores an array of frames. Each frame is skeleton pose in local space.
 It is important that animation STATE is not stored in the anim so that multiple
 meshes can share the same animation DATA.
 */

typedef struct
{
    unsigned short jointCount;
    unsigned short frameCount;
    unsigned short framesPerSecond;
    unsigned short markerCount;
    
    SkelAnimFrame* frames;
    SkelAnimJoint* frameData;
    SkelAnimMarker* markers;
        
} SkelAnim;

extern int SkelAnim_Init(SkelAnim* anim,
                         unsigned short jointCount,
                         unsigned short frameCount,
                         unsigned short markerCount);
extern void SkelAnim_Shutdown(SkelAnim* anim);

extern int SkelAnim_FromPath(SkelAnim* anim, const char* path);
extern int SkelAnim_FindMarker(SkelAnim* anim, const char* markerName);

#define SkelAnim_RandomFrame(anim) (rand() % ((anim)->frameCount))


/* a report of the current animation state */
typedef struct
{
    /* is there a marker at this frame? */
    SkelAnimMarker* marker;
    /* is this frame the end of the animation? */
    int finishedAnim;
    int finishedTransition;
} SkelAnimatorInfo;



/*
DESIGN
 =================================
 - animation transitions
 - Don't want to lose ability to "just play an animation"
 
 IDEAS
 =================================
 - Blending/State trees
 - Root bone motion https://udn.epicgames.com/Three/RootMotion.htm
 */


typedef struct
{
    Skel* skel;
    const SkelAnim* anim;
    const SkelAnim* targetAnim;
    short targetStartFrame;
    
    short frame;
    short subFrame;
    
    short transitionFrameCount;
    short transitionFrame;
    
    int playbackRate;
        
    /* frames between interval will be interpolated when enabled. When disabled the nearest frame will be selected */
    int enableInterp;
} SkelAnimator;

extern int SkelAnimator_Init(SkelAnimator* animator, Skel* skel);
extern void SkelAnimator_Shutdown(SkelAnimator* animator);


// returns 1 if the animation changed, 0 if it remained the same.
extern int SkelAnimator_SetAnim(SkelAnimator* animator,
                                 const SkelAnim* anim,
                                 int startFrame,
                                 int transitionFrameCount);

extern int SkelAnimator_InTransition(SkelAnimator* animator);

extern void SkelAnimator_RestartAnim(SkelAnimator* animator);

extern SkelAnimatorInfo SkelAnimator_Tick(SkelAnimator* animator);

#endif
