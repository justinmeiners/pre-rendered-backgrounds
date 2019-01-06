
#include "material.h"
#include <memory.h>

void Material_Init(Material* material)
{
    material->albedoMap = -1;
    material->normalMap = -1;
    material->specularMap = -1;
}

void Material_Copy(Material* dest, const Material* source)
{
    memcpy(dest, source, sizeof(Material));
}
