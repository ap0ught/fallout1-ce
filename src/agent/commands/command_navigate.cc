#include "command_navigate.h"
#include "agent/agent_commands.h"

#include <cstdint>
#include <SDL_net.h>

#include "game/gmouse.h"
#include "plib/gnw/debug.h"
#include "plib/gnw/input.h"
#include "plib/gnw/dxinput.h"

namespace fallout {

void handle_command_navigate(unsigned char* msg, MouseData* mouseData)
{
    gmouse_3d_set_mode(GAME_MOUSE_MODE_MOVE);
    gmouse_set_cursor(MOUSE_CURSOR_NONE);

    uint8_t action = msg[0];
    KeyboardData keyboardData;

    switch (action) {
    case ACTION_SCROLL_LEFT:
        agent_simulate_key(SDL_SCANCODE_LEFT);
        break;
    case ACTION_SCROLL_RIGHT:
        agent_simulate_key(SDL_SCANCODE_RIGHT);
        break;
    case ACTION_SCROLL_UP:
        agent_simulate_key(SDL_SCANCODE_UP);
        break;
    case ACTION_SCROLL_DOWN:
        agent_simulate_key(SDL_SCANCODE_DOWN);
        break;
    case ACTION_CENTER:
        agent_simulate_key(SDL_SCANCODE_HOME);
        break;
    case ACTION_RUN:
        keyboardData.key = SDL_SCANCODE_LSHIFT;
        keyboardData.down = true;
        agent_process_key(&keyboardData);
        
        gAgentLShiftHeld = true;

        [[fallthrough]];
    case ACTION_WALK:
        uint32_t x = SDLNet_Read32(msg + 1);
        uint32_t y = SDLNet_Read32(msg + 5);

        mouseData->x = x;
        mouseData->y = y;
        mouseData->buttons[0] = 1;
        mouse_get_agent_delta(mouseData);

        gAgentCommandsUpdateDelay = 1;

        break;
    }
}

} // namespace fallout
