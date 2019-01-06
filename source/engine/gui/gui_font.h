
#ifndef GUI_FONT_H
#define GUI_FONT_H

#include "vec_math.h"

/* Bitmap font - Each character is evenly spaced on a spritesheet */

#define FONT_CHAR_MAX 512

typedef struct
{
    Vec2 uvs[FONT_CHAR_MAX];
    float charSize;
    
    Vec2 charOffsets[FONT_CHAR_MAX];
} GuiFont;

/* chars per row = 16 */
/* char count = 256 */

extern int GuiFont_Init(GuiFont* font, int charsPerRow, int charCount);
extern void GuiFont_Shutdown(GuiFont* font);


#endif
