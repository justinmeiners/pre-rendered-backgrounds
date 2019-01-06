

#ifndef SKEL_MODEL_H
#define SKEL_MODEL_H

#include "skel_anim.h"
#include "skel_skin.h"
#include "material.h"

/*
 
 Each frame the SkelModel selectes the last and next frame from its anim, interpolates
 animation state between that too and applies it as the current local pose.
 World space skeleton is then calculated and applied to vertices.
 
 
 Inspired by:
 http://tfc.duke.free.fr/coding/md5-specs-en.html
 
 */

typedef struct
{
    Skel skel;
    SkelSkin skin;
    SkelAnimator animator;
    
    // for saving lookup indicies into attach points
    // this is in here, instead of the skel to keep the skel pure
    short attachPointTable[SKEL_ATTACH_POINTS_MAX];
    
    Material material;

} SkelModel;

extern int SkelModel_Copy(SkelModel* dest, const SkelModel* source);
extern int SkelModel_FromPath(SkelModel* model, const char* path);

extern void SkelModel_Shutdown(SkelModel* model);

extern SkelAnimatorInfo SkelModel_Tick(SkelModel* model);

extern const SkelAttachPoint* SkelModel_AttachPointAt(const SkelModel* model, int loc);

#endif
