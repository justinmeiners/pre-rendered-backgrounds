
#include "gui_buffer.h"

void GuiBuffer_Init(GuiBuffer* buffer)
{
    buffer->vaoGpuId = 0;
    buffer->iboGpuId = 0;
    buffer->vboGpuId = 0;
    
    buffer->indexCount = 0;
    buffer->vertCount = 0;
}

void GuiBuffer_Clear(GuiBuffer* buffer)
{
    buffer->vertCount = 0;
    buffer->indexCount = 0;
}

