
#include "gui_font.h"

int GuiFont_Init(GuiFont* font, int charsPerRow, int charCount)
{
    if (font)
    {
        /* figure out UV coordinates for each ASCII character */
        const float fCharsPerRow = (float)charsPerRow;
        
        float cx, cy;
        const float squareSize = 1.0f / fCharsPerRow;
        int i;
        int index;
        for (i = 0; i < charCount; ++i)
        {
            cx = (i % charsPerRow) / fCharsPerRow;
            cy =  (i / charsPerRow) / fCharsPerRow;
            
            index = i * 2;
            
            font->uvs[index].x = cx;
            font->uvs[index].y = cy + squareSize;
            
            font->uvs[index + 1].x = cx + squareSize;
            font->uvs[index + 1].y = cy;
            
            font->charOffsets[i] = Vec2_Zero;
        }
    }
    
    return 1;
}

void GuiFont_Shutdown(GuiFont* font)
{

}
