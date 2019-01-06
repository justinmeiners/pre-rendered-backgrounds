
#ifndef GUI_LABEL_H
#define GUI_LABEL_H

#include "vec_math.h"


typedef enum
{
    kTextAlignLeft = 0,
    kTextAlignRight,
    kTextAlignCenter
} TextAlign;

typedef enum
{
    kTextFaceNormal = 0,
    kTextFaceBold = 1,
} TextFace;

#define GUI_LABEL_TEXT_MAX 256

typedef struct
{
    char text[GUI_LABEL_TEXT_MAX];
    int textLength;
    
    Vec4 textColor;
    TextAlign align;
    float textSize;
    TextFace face;
    
    /* normalied position within owning view. Eg. 0.5, 0.5 is centered */
    Vec2 anchor;
} GuiLabel;

extern void GuiLabel_Init(GuiLabel* label);
extern void GuiLabel_Shutdown(GuiLabel* label);

extern void GuiLabel_SetText(GuiLabel* label, const char* string);
extern void GuiLabel_ClearText(GuiLabel* label);

#endif
