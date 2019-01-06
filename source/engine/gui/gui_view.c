
#include "gui_view.h"

static void GuiViewState_Init(struct GuiViewState* state, GuiViewState type)
{
    if (type == kGuiViewStateNormal)
    {
        state->_uvEnabled = 1;
        state->_colorEnabled = 1;
    }
    else
    {
        state->_uvEnabled = 0;
        state->_colorEnabled = 0;
    }
    
    state->uvs[0] = Vec2_Create(0.0f, 0.0f);
    state->uvs[1] = Vec2_Create(1.0f, 0.0f);
    state->uvs[2] = Vec2_Create(1.0f, 1.0f);
    state->uvs[3] = Vec2_Create(0.0f, 1.0f);
    
    state->colors[0] = Vec4_Create(1.0f, 1.0f, 1.0f, 1.0f);
    state->colors[1] = state->colors[0];
    state->colors[2] = state->colors[0];
    state->colors[3] = state->colors[0];
}


int GuiView_Init(GuiView* view)
{
    if (!view)
        return 0;
    
    view->tag = -1;
    
    view->dead = 1;

    view->origin = Vec2_Zero;
    view->size = Vec2_Zero;
    view->state = kGuiViewStateNormal;
    
    view->pressed = 0;
    view->pressEnabled = 0;
    view->drawViewEnabled = 1;
    
    view->blocksTouch = 1;
        
    int i;
    for (i = 0; i < kGuiViewStateCount; ++i)
    {
        GuiViewState_Init(&view->_states[i], kGuiViewStateNormal + i);
    }
    
    view->parent = NULL;
    
    for (i = 0; i < LAYER_CHILDREN_MAX; ++i)
    {
        view->children[i] = NULL;
    }
    
    view->childCount = 0;
    
    view->angle = 0.0f;
    view->hidden = 0;
        
    view->atlas = kGuiAtlasSkin;
    
    GuiLabel_Init(&view->label);
    return 1;
}

int GuiView_AddChild(GuiView* parent, GuiView* child)
{
    if (parent->childCount + 1 < LAYER_CHILDREN_MAX)
    {
        child->parent = parent;
        
        parent->children[parent->childCount] = child;
        ++parent->childCount;
        return 1;
    }
    
    return 0;
}

void GuiView_RemoveChildren(GuiView* view)
{
    int i;
    for (i = 0; i < LAYER_CHILDREN_MAX; ++i)
    {
        if (view->children[i] != NULL)
        {
            view->children[i]->parent = NULL;
        }
        
        view->children[i] = NULL;
    }
    
    view->childCount = 0;
}

int GuiView_Intersects(const GuiView* view, Vec2 point)
{
    if (point.x < view->_screenOrigin.x ||
        point.y < view->_screenOrigin.y ||
        point.x > view->_screenOrigin.x + view->size.x ||
        point.y > view->_screenOrigin.y + view->size.y)
    {
        return 0;
    }
    
    return 1;
}

GuiView* GuiView_HitTest_r(const GuiView* view, Vec2 point)
{
    int i = 0;
    for (i = view->childCount - 1; i >= 0; --i)
    {
        const GuiView* child = view->children[i];
        
        if (child->dead || child->hidden) { continue; }
        
        if (GuiView_Intersects(child, point))
        {
            return GuiView_HitTest_r(child, point);
        }
    }
    
    return (GuiView*)view;
}

void GuiView_SetUvs(GuiView* view, Vec2 origin, Vec2 size, Vec2 normal, GuiViewState state)
{
    int i = 0;
    for (i = 0; i < kGuiViewStateCount; ++i)
    {
        struct GuiViewState* current = view->_states + i;
        
        if (i == state)
        {
            current->_uvEnabled = 1;
        }
        
        if (i == state || current->_uvEnabled == 0)
        {
            current->uvs[0] = Vec2_Create(origin.x / normal.x, origin.y / normal.y);
            current->uvs[1] = Vec2_Create((origin.x + size.x) / normal.x, origin.y / normal.y);
            current->uvs[2] = Vec2_Create((origin.x + size.x) / normal.x, (origin.y + size.y) / normal.y);
            current->uvs[3] = Vec2_Create(origin.x / normal.x, (origin.y + size.y) / normal.y);
        }
    }
}

void GuiView_SetUvsRotated(GuiView* view, Vec2 origin, Vec2 size, Vec2 normal, GuiViewState state)
{
    int i = 0;
    for (i = 0; i < kGuiViewStateCount; ++i)
    {
        struct GuiViewState* current = view->_states + i;
        
        if (i == state)
        {
            current->_uvEnabled = 1;
        }
        
        if (i == state || current->_uvEnabled == 0)
        {
            current->uvs[0] = Vec2_Create(origin.x / normal.x, (origin.y + size.y) / normal.y);
            current->uvs[1] = Vec2_Create(origin.x / normal.x, origin.y / normal.y);
            current->uvs[2] = Vec2_Create((origin.x + size.x) / normal.x, origin.y / normal.y);
            current->uvs[3] = Vec2_Create((origin.x + size.x) / normal.x, (origin.y + size.y) / normal.y);
        }
    }
}

void GuiView_SetColor(GuiView* view, Vec4 color, GuiViewState state)
{
    int i = 0;
    for (i = 0; i < kGuiViewStateCount; ++i)
    {
        struct GuiViewState* current = view->_states + i;
        
        if (i == state)
        {
            current->_colorEnabled = 1;
        }
        
        if (i == state || current->_colorEnabled == 0)
        {
            current->colors[0] = color;
            current->colors[1] = color;
            current->colors[2] = color;
            current->colors[3] = color;
        }
    }
}

void GuiView_ClearState(GuiView* view, GuiViewState st)
{
    struct GuiViewState* state = &view->_states[st];
    
    state->_colorEnabled = 0;
    state->_uvEnabled = 0;
}

void GuiView_SetEnabled(GuiView* view, int enabled)
{
    if (enabled)
    {
        view->state = kGuiViewStateNormal;
    }
    else
    {
        view->state = kGuiViewStateDisabled;
    }
}
