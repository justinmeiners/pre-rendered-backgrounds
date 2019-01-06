
#ifndef STATIC_MODEL_H
#define STATIC_MODEL_H

#include "static_mesh.h"
#include "material.h"

typedef struct
{
    StaticMesh mesh;
    Material material;
} StaticModel;

extern int StaticModel_FromPath(StaticModel* model, const char* path);
extern int StaticModel_Copy(StaticModel* dest, const StaticModel* source);

extern void StaticModel_Shutdown(StaticModel* model);

#endif
