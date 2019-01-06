
#ifndef GUI_VIEW_H
#define GUI_VIEW_H

#include "gui_label.h"

typedef enum
{
    kGuiAtlasNone = -1,
    kGuiAtlasSkin = 0,
    kGuiAtlasFont,
    kGuiAtlasItems,
    kGuiAtlasInfo,
} GuiTexture;

typedef enum
{
    kGuiViewStateNormal = 0,
    kGuiViewStateDisabled,
    kGuiViewStatePress,
    kGuiViewStateHover,
    kGuiViewStateSelected,
    kGuiViewStateCount
} GuiViewState;



/* Inspired by iOS UIKit */
struct GuiViewState
{
    Vec2 uvs[4];
    Vec4 colors[4];
    int _colorEnabled;
    int _uvEnabled;
};

#define LAYER_CHILDREN_MAX 32

typedef struct GuiView
{
    /* user assigned identifier */
    int tag;
    int dead;
    
    GuiViewState state;
    
    struct GuiViewState _states[kGuiViewStateCount];
    
    struct GuiView* parent;
    struct GuiView* children[LAYER_CHILDREN_MAX];
    int childCount;
    
    
    /* absolute coordinate relative to root gui layer */
    Vec2 _screenOrigin;
    
    /* relative to parent */
    Vec2 origin;
    Vec2 size;
    
    int hidden;
    float angle;
    
    GuiLabel label;
    
    int pressed;
    int _pressIdentifier;
    int pressEnabled;
    
    int blocksTouch;
    
    int drawViewEnabled;
            
    int atlas;

} GuiView;

extern int GuiView_Init(GuiView* view);

extern int GuiView_AddChild(GuiView* parent, GuiView* child);
extern void GuiView_RemoveChildren(GuiView* parent);

/* the normal state will be used unless a property has been set on an alternate state */
/* assignment will affect all unassigned states */
extern void GuiView_SetUvs(GuiView* view, Vec2 origin, Vec2 size, Vec2 normal, GuiViewState state);

/* for when the UVs are rotated 90 degrees clockwise in the texture */
extern void GuiView_SetUvsRotated(GuiView* view, Vec2 origin, Vec2 size, Vec2 normal, GuiViewState state);

extern void GuiView_SetColor(GuiView* view, Vec4 color, GuiViewState state);

/* this will clear a state so it defaults to the normal state */
extern void GuiView_ClearState(GuiView* view, GuiViewState state);

extern void GuiView_SetEnabled(GuiView* view, int enabled);
extern int GuiView_Intersects(const GuiView* view, Vec2 point);
extern GuiView* GuiView_HitTest_r(const GuiView* view, Vec2 point);

#endif
