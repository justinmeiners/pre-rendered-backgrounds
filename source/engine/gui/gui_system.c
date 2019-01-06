
#include "gui_system.h"
#include <limits.h>

int GuiSystem_Init(GuiSystem* system, short guiWidth, short guiHeight)
{
    if (!system)
        return 0;
    
    system->guiWidth = guiWidth;
    system->guiHeight = guiHeight;
    
    int i;
    for (i = 0; i < GUI_SYSTEM_LAYERS_MAX; ++i)
    {
        GuiView* view = system->views + i;
        view->dead = 1;
    }
    
    system->onEvent = NULL;
    system->onTick = NULL;
    
    system->drawEnabled = 1;
    
    GuiFont_Init(&system->font, 16, 256);
    
    system->root = GuiSystem_SpawnView(system, NULL);
    system->root->drawViewEnabled = 0;
    system->root->blocksTouch = 0;
    
    GuiBuffer_Init(&system->buffer);
    
    return 1;
}

void GuiSystem_Shutdown(GuiSystem* system)
{
    
}

static GuiVertColor GuiColor_FromVec4(Vec4 v)
{
    GuiVertColor c;
    c.r = 255 * v.x;
    c.g = 255 * v.y;
    c.b = 255 * v.z;
    c.a = 255 * v.w;
    return c;
}

static void Gui_PackLabel(const GuiLabel* label,
                           const GuiFont* font,
                           Vec2 origin,
                           GuiBuffer* buffer)
{
    const int strLength = label->textLength;
    
    int i = 0;
    
    Vec4 color = label->textColor;
    char atlas = (char)kGuiAtlasFont;
    
    Vec2 position = origin;
    
    Vec2 uvmin;
    Vec2 uvmax;
    
    Vec2 min;
    Vec2 max;
    
    while (i < strLength)
    {
        /* find how long this string segment is */
        int end = i;
        while (end < strLength)
        {
            if (label->text[end] == '\\')
            {
                break;
            }
            
            end++;
        }
        int sublength = end-i;
        
        
        /* do some align */
        float stringSize = sublength * label->textSize;
        
        if (label->align == kTextAlignCenter)
        {
            position.x = origin.x -stringSize / 2.0;
        }
        else if (label->align == kTextAlignRight)
        {
            position.x = origin.x - stringSize;
        }
        else
        {
            position.x = origin.x;
        }
        
        
        /* draw the substring */
        int k;
        for (k = 0; k < sublength; ++k)
        {
            char character = label->text[i + k] - 32;
            
            uvmin = font->uvs[character * 2];
            uvmax = font->uvs[character * 2 + 1];
            
            const Vec2 charOffset = font->charOffsets[character];
            
            min = Vec2_Add(charOffset, Vec2_Create(position.x + k * label->textSize, position.y));
            max = Vec2_Add(charOffset, Vec2_Create(position.x + k * label->textSize + label->textSize, position.y + label->textSize));
            
            /*
              ___
             |\  |
             | \ |
             |__\|
             
             left triangle
             (min, min) -> (max, min) -> (min, max)
             
             right triangle
             (min, max) -> (max, min) -> (max, max)
             */
            
            buffer->indicies[buffer->indexCount] = buffer->vertCount;
            buffer->indicies[buffer->indexCount + 1] = buffer->vertCount + 1;
            buffer->indicies[buffer->indexCount + 2] = buffer->vertCount + 3;
            
            buffer->indicies[buffer->indexCount + 3] = buffer->vertCount + 3;
            buffer->indicies[buffer->indexCount + 4] = buffer->vertCount + 1;
            buffer->indicies[buffer->indexCount + 5] = buffer->vertCount + 2;
            buffer->indexCount += 6;
            
            buffer->verts[buffer->vertCount].atlasId = atlas;
            buffer->verts[buffer->vertCount].color = GuiColor_FromVec4(color);
            buffer->verts[buffer->vertCount].pos.x = min.x;
            buffer->verts[buffer->vertCount].pos.y = min.y;
            buffer->verts[buffer->vertCount].uv.u = ceilf(USHRT_MAX * uvmin.x);
            buffer->verts[buffer->vertCount].uv.v = ceilf(USHRT_MAX * uvmin.y);
            ++buffer->vertCount;
            
            buffer->verts[buffer->vertCount].atlasId = atlas;
            buffer->verts[buffer->vertCount].color = GuiColor_FromVec4(color);
            buffer->verts[buffer->vertCount].pos.x = max.x;
            buffer->verts[buffer->vertCount].pos.y = min.y;
            buffer->verts[buffer->vertCount].uv.u = ceilf(USHRT_MAX * uvmax.x);
            buffer->verts[buffer->vertCount].uv.v = ceilf(USHRT_MAX * uvmin.y);
            ++buffer->vertCount;
            
            buffer->verts[buffer->vertCount].atlasId = atlas;
            buffer->verts[buffer->vertCount].color = GuiColor_FromVec4(color);
            buffer->verts[buffer->vertCount].pos.x = max.x;
            buffer->verts[buffer->vertCount].pos.y = max.y;
            buffer->verts[buffer->vertCount].uv.u = ceilf(USHRT_MAX * uvmax.x);
            buffer->verts[buffer->vertCount].uv.v = ceilf(USHRT_MAX * uvmax.y);
            ++buffer->vertCount;
            
            buffer->verts[buffer->vertCount].atlasId = atlas;
            buffer->verts[buffer->vertCount].color = GuiColor_FromVec4(color);
            buffer->verts[buffer->vertCount].pos.x = min.x;
            buffer->verts[buffer->vertCount].pos.y = max.y;
            buffer->verts[buffer->vertCount].uv.u = ceilf(USHRT_MAX * uvmin.x);
            buffer->verts[buffer->vertCount].uv.v = ceilf(USHRT_MAX * uvmax.y);
            ++buffer->vertCount;
        }
        
        position.y -= label->textSize;
        
        i = end + 1;
    }
}

static void Gui_PackGuiView(const GuiView* view,
                             GuiBuffer* buffer)
{
    if (view->atlas == kGuiAtlasNone)
    {
        return;
    }
    
    const struct GuiViewState* state = view->_states + view->state;
    
    /*
     __
     |\  |
     | \ |
     |__\|
     
     left triangle
     (min, min) -> (max, min) -> (min, max)
     
     right triangle
     (min, max) -> (max, min) -> (max, max)
     */
    
    buffer->indicies[buffer->indexCount] = buffer->vertCount;
    buffer->indicies[buffer->indexCount + 1] = buffer->vertCount + 1;
    buffer->indicies[buffer->indexCount + 2] = buffer->vertCount + 3;
    
    buffer->indicies[buffer->indexCount + 3] = buffer->vertCount + 3;
    buffer->indicies[buffer->indexCount + 4] = buffer->vertCount + 1;
    buffer->indicies[buffer->indexCount + 5] = buffer->vertCount + 2;
    buffer->indexCount += 6;
    
    char atlas = (char)view->atlas;
    
    buffer->verts[buffer->vertCount].atlasId = atlas;
    buffer->verts[buffer->vertCount].pos = view->_screenOrigin;
    buffer->verts[buffer->vertCount].uv.u = ceilf(USHRT_MAX * state->uvs[0].x);
    buffer->verts[buffer->vertCount].uv.v = ceilf(USHRT_MAX * state->uvs[0].y);
    buffer->verts[buffer->vertCount].color = GuiColor_FromVec4(state->colors[0]);
    ++buffer->vertCount;
    
    buffer->verts[buffer->vertCount].atlasId = atlas;
    buffer->verts[buffer->vertCount].pos = Vec2_Offset(view->_screenOrigin, view->size.x, 0.0f);
    buffer->verts[buffer->vertCount].uv.u = ceilf(USHRT_MAX * state->uvs[1].x);
    buffer->verts[buffer->vertCount].uv.v = ceilf(USHRT_MAX * state->uvs[1].y);
    buffer->verts[buffer->vertCount].color = GuiColor_FromVec4(state->colors[1]);
    ++buffer->vertCount;
    
    buffer->verts[buffer->vertCount].atlasId = atlas;
    buffer->verts[buffer->vertCount].pos = Vec2_Offset(view->_screenOrigin, view->size.x, view->size.y);
    buffer->verts[buffer->vertCount].uv.u = ceilf(USHRT_MAX * state->uvs[2].x);
    buffer->verts[buffer->vertCount].uv.v = ceilf(USHRT_MAX * state->uvs[2].y);
    buffer->verts[buffer->vertCount].color = GuiColor_FromVec4(state->colors[2]);
    ++buffer->vertCount;
    
    buffer->verts[buffer->vertCount].atlasId = atlas;
    buffer->verts[buffer->vertCount].pos = Vec2_Offset(view->_screenOrigin, 0.0f, view->size.y);
    buffer->verts[buffer->vertCount].uv.u = ceilf(USHRT_MAX * state->uvs[3].x);
    buffer->verts[buffer->vertCount].uv.v = ceilf(USHRT_MAX * state->uvs[3].y);
    buffer->verts[buffer->vertCount].color = GuiColor_FromVec4(state->colors[3]);
    ++buffer->vertCount;
}


static void GuiSystem_Traverse_r(GuiSystem* system, GuiView* current)
{
    if (current->parent)
    {
        current->_screenOrigin = Vec2_Add(current->parent->_screenOrigin, current->origin);
    }
    else
    {
        current->_screenOrigin = current->origin;
    }
    
    if (!current->hidden)
    {
        if (current->drawViewEnabled)
        {
            Gui_PackGuiView(current, &system->buffer);
        }
        
        if (current->label.textLength > 0)
        {
            Vec2 origin = Vec2_Mult(current->size, current->label.anchor);
            origin = Vec2_Add(origin, current->_screenOrigin);
            
            Gui_PackLabel(&current->label, &system->font, origin, &system->buffer);
        }
        
        int i;
        for (i = 0; i < current->childCount; ++i)
        {
            GuiView* child = current->children[i];
            
            if (child->dead)
            {
                continue;
            }
            
            GuiSystem_Traverse_r(system, child);
        }
    }

}

void GuiSystem_Tick(GuiSystem* system)
{
    system->onTick(system, system->delegate);
    
    GuiBuffer_Clear(&system->buffer);
    
    if (system->drawEnabled)
    {
        GuiSystem_Traverse_r(system, system->root);
    }
}

void GuiSystem_ReceiveInput(GuiSystem* system,
                            const InputState* state,
                            const InputState* last,
                            InputInfo* info)
{
    int i;
    for (i = 0; i < GUI_SYSTEM_LAYERS_MAX; ++i)
    {
        GuiView* view = system->views + i;
        
        if (view->pressed && !view->dead)
        {
            if (state->mouseButtons[kMouseButtonLeft].phase == kInputPhaseEnd)
            {
                Vec2 point = GuiSystem_ProjectNormalized(system, Vec2_Create(state->mouseCursor.x, state->mouseCursor.y));
                
                if (system->onEvent && GuiView_Intersects(view, point))
                {
                    GuiEvent newEvent;
                    newEvent.viewTag = view->tag;
                    newEvent.type = kGuiEventGuiViewTap;
                    system->onEvent(system, &newEvent, system->delegate);
                }
                
                view->pressed = 0;
                view->state = kGuiViewStateNormal;
                            }
            else
            {
                int j;
                for (j = 0; j < state->touchCount; ++j)
                {
                    if (state->touches[j].identifier == view->_pressIdentifier &&
                        info->touches[j].handled &&
                        state->touches[j].phase == kInputPhaseEnd)
                    {
                        Vec2 point = GuiSystem_ProjectNormalized(system, Vec2_Create(state->touches[j].x, state->touches[j].y));
                        
                        if (system->onEvent && GuiView_Intersects(view, point))
                        {
                            GuiEvent newEvent;
                            newEvent.viewTag = view->tag;
                            newEvent.type = kGuiEventGuiViewTap;
                            system->onEvent(system, &newEvent, system->delegate);
                        }
                        
                        view->pressed = 0;
                        view->state = kGuiViewStateNormal;
                        break;
                    }
                }
            }
        }
    }
    
    if (state->mouseButtons[kMouseButtonLeft].phase == kInputPhaseBegin && !info->mouseButtons[kMouseButtonLeft].handled)
    {
        GuiView* view = GuiView_HitTest_r(system->root, GuiSystem_ProjectNormalized(system, Vec2_Create(state->mouseCursor.x, state->mouseCursor.y)));
        
        if (view && view != system->root)
        {
            if (view->state != kGuiViewStateDisabled && view->pressEnabled)
            {
                view->pressed = 1;
                view->state = kGuiViewStatePress;
            }
            
            if (view->blocksTouch)
            {
                info->mouseButtons[kMouseButtonLeft].handled = 1;
            }
        }
    }
    else
    {
        int j;
        for (j = 0; j < state->touchCount; ++j)
        {
            if (state->touches[j].phase == kInputPhaseBegin && !info->touches[j].handled)
            {
                GuiView* view = GuiView_HitTest_r(system->root, GuiSystem_ProjectNormalized(system, Vec2_Create(state->touches[j].x, state->touches[j].y)));
                
                if (view && view != system->root)
                {
                    if (view->state != kGuiViewStateDisabled && view->pressEnabled)
                    {
                        view->pressed = 1;
                        view->_pressIdentifier = state->touches[j].identifier;
                        view->state = kGuiViewStatePress;
                    }
                    
                    if (view->blocksTouch)
                    {
                        info->touches[j].handled = 1;
                    }
                }
            }
        }
    }
}

GuiView* GuiSystem_SpawnView(GuiSystem* system, GuiView* parent)
{
    int i;
    for (i = 0; i < GUI_SYSTEM_LAYERS_MAX; ++i)
    {
        GuiView* view = system->views + i;
        
        if (view->dead)
        {
            GuiView_Init(view);
            view->dead = 0;
            
            if (parent)
            {
                GuiView_AddChild(parent, view);
            }
            
            return view;
        }
    }
    
    return NULL;
}

GuiView* GuiSystem_FindView(GuiSystem* system, int tag)
{
    int i;
    for (i = 0; i < GUI_SYSTEM_LAYERS_MAX; ++i)
    {
        GuiView* view = system->views + i;
        
        if (view->dead) { continue; }
        
        if (view->tag == tag)
        {
            return view;
        }
    }
    return NULL;
}

Vec2 GuiSystem_ProjectNormalized(GuiSystem* system, Vec2 point)
{
    const Vec2 offset = Vec2_Scale(Vec2_Offset(point, 1.0f, 1.0f), 0.5f);
    return Vec2_Create(system->guiWidth * offset.x, system->guiHeight * offset.y);
}

Vec2 GuiSystem_Project(GuiSystem* system, Vec2 point, short surfaceWidth, short surfaceHeight)
{
    const Vec2 divided = Vec2_Create(point.x / (float)surfaceWidth, point.y / (float)surfaceHeight);
    const Vec2 projected = Vec2_Create(system->guiWidth * divided.x, system->guiHeight * divided.y);
    
    return projected;
}
