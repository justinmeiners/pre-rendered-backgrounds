
#ifndef GUI_SYSTEM_H
#define GUI_SYSTEM_H

#include "gui_view.h"
#include "gui_font.h"
#include "input_system.h"
#include "gui_buffer.h"

#define GUI_SYSTEM_LAYERS_MAX 64

typedef enum
{
    kGuiCursorNormal = 0,
    
} GuiCursorType;

typedef struct
{
    Vec2 origin;
    Vec2 size;
    
    GuiCursorType type;
} GuiCursor;

typedef enum
{
    kGuiEventGuiViewTap = 0,
} GuiEventType;

typedef struct
{
    GuiEventType type;
    int viewTag;
} GuiEvent;

typedef struct GuiSystem
{
    int guiWidth;
    int guiHeight;
    
    GuiCursorType cursor;
    
    GuiBuffer buffer;
    
    int drawEnabled;
    
    GuiView* root;
    
    GuiView views[GUI_SYSTEM_LAYERS_MAX];
    GuiFont font;
    
    void* delegate;
    void (*onEvent)(struct GuiSystem* system, const GuiEvent* event, void* delegate);
    void (*onTick)(struct GuiSystem* system, void* delegate);
} GuiSystem;

extern int GuiSystem_Init(GuiSystem* system, short guiWidth, short guiHeight);
extern void GuiSystem_Shutdown(GuiSystem* system);

extern void GuiSystem_Tick(GuiSystem* system);

extern void GuiSystem_ReceiveInput(GuiSystem* system,
                                   const InputState* state,
                                   const InputState* last,
                                   InputInfo* info);

extern GuiView* GuiSystem_SpawnView(GuiSystem* system, GuiView* parent);
extern GuiView* GuiSystem_FindView(GuiSystem* system, int tag);

extern Vec2 GuiSystem_ProjectNormalized(GuiSystem* system, Vec2 point);
extern Vec2 GuiSystem_Project(GuiSystem* system, Vec2 point, short surfaceWidth, short surfaceHeight);

#endif
