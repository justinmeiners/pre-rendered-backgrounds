
#ifndef MATERIAL_H
#define MATERIAL_H

typedef enum
{
    kMaterialNone = 0,
    kMaterialCharacter,
    kMaterialProp,
} MaterialType;

typedef struct
{
    MaterialType type;

    int albedoMap;
    int normalMap;
    int specularMap;
    int glossMap;
    
} Material;

extern void Material_Init(Material* material);

extern void Material_Copy(Material* dest, const Material* source);
#endif
