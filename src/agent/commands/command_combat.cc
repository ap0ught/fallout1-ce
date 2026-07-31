#include "command_combat.h"
#include "agent/agent_commands.h"

#include <cstdint>
#include <SDL_net.h>

#include "game/gmouse.h"
#include "plib/gnw/input.h"
#include "plib/gnw/dxinput.h"

namespace fallout {

void handle_command_combat(unsigned char* msg, MouseData* mouseData)
{
    uint8_t action = msg[0];

    switch (action) {
    case ACTION_BEGIN:
        agent_simulate_key(SDL_SCANCODE_A);
        break;
    case ACTION_END_TURN:
        agent_simulate_key(SDL_SCANCODE_SPACE);
        break;
    case ACTION_END_COMBAT:
        agent_simulate_key(SDL_SCANCODE_KP_ENTER);
        break;
    case ACTION_ATTACK:
        uint32_t x = SDLNet_Read32(msg + 1);
        uint32_t y = SDLNet_Read32(msg + 5);

        gmouse_3d_set_mode(GAME_MOUSE_MODE_CROSSHAIR);
        gmouse_set_cursor(MOUSE_CURSOR_CROSSHAIR);

        mouseData->x = x;
        mouseData->y = y;
        mouseData->buttons[0] = 1;
        mouse_get_agent_delta(mouseData);
    }
}

} // namespace fallout
