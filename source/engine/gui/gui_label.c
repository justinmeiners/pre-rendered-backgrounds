
#include "gui_label.h"
#include <string.h>

void GuiLabel_Init(GuiLabel* label)
{
    memset(label->text, 0x0, sizeof(label->text));
    label->textColor = Vec4_Create(1.0f, 1.0f, 1.0f, 1.0f);
    label->align = kTextAlignCenter;
    label->textSize = 18.0f;
    label->face = kTextFaceNormal;
    
    label->textLength = 0;
    
    label->anchor = Vec2_Zero;
}


void GuiLabel_Shutdown(GuiLabel* label)
{
}

void GuiLabel_SetText(GuiLabel* label, const char* string)
{
    strcpy(label->text, string);
    label->textLength = (int)strnlen(string, GUI_LABEL_TEXT_MAX);
}

void GuiLabel_ClearText(GuiLabel* label)
{
    label->textLength = 0;
}
