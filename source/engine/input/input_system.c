
#include "input_system.h"

void InputState_Init(InputState* state)
{
    memset(state, 0, sizeof(InputState));
}

int InputSystem_Init(InputSystem* system, InputConfig config)
{
    memset(system, 0x0, sizeof(InputSystem));
    system->config = config;
    
    system->panIdentifier1 = -1;
    system->panIdentifier2 = -1;

    return 1;
}

void InputSystem_ProcessInput(InputSystem* system, const InputState* newState)
{
    system->last = system->current;
    memcpy(&system->current, newState, sizeof(InputState));
    
    
    InputState* state = &system->current;
    const InputState* last = &system->last;
    
    int i;
    for (i = 0; i < kMouseButtonCount; ++i)
    {
        if (state->mouseButtons[i].down && !last->mouseButtons[i].down)
        {
            state->mouseButtons[i].phase = kInputPhaseBegin;
            system->info.mouseButtons[i].handled = 0;
        }
        else if (!state->mouseButtons[i].down && last->mouseButtons[i].down)
        {
            state->mouseButtons[i].phase = kInputPhaseEnd;
        }
        else
        {
            state->mouseButtons[i].phase = kInputPhaseMove;
        }
    }
    
    for (i = 0; i < INPUT_TOUCHES_MAX; ++i)
    {
        if (state->touches[i].identifier == last->touches[i].identifier)
        {
            if (state->touches[i].phase == kInputPhaseEnd &&
                (last->touches[i].phase == kInputPhaseEnd || last->touches[i].phase == kInputPhaseNone))
            {
                state->touches[i].phase = kInputPhaseNone;
            }
            
            if (state->touches[i].phase == kInputPhaseBegin)
            {
                system->info.touches[i].handled = 0;
            }
        }
    }
}

