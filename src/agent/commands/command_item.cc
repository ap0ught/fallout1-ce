#include "command_item.h"

#include <SDL.h>

#include "plib/gnw/input.h"

namespace fallout {

void handle_command_item(unsigned char* msg)
{
    uint8_t action = msg[0];

    switch (action) {
    case ACTION_CHANGE_HAND:
        agent_simulate_key(SDL_SCANCODE_B);
        break;
    case ACTION_CHANGE_MODE:
        agent_simulate_key(SDL_SCANCODE_N);
        break;
    }
}

} // namespace fallout
