#include "command_save.h"

#include <cstdio>
#include <cstring>

#include "agent/agent_ipc.h"
#include "plib/gnw/input.h"
#include "plib/gnw/debug.h"

namespace fallout {

void handle_command_save(unsigned char* msg, uint32_t length)
{
    uint8_t action = msg[0];

    switch (action) {
    case ACTION_SAVE:
        agent_simulate_key(SDL_SCANCODE_F4);
        break;
    case ACTION_QUICK_SAVE:
        agent_simulate_key(SDL_SCANCODE_F6);
        break;
    case ACTION_LOAD:
        agent_simulate_key(SDL_SCANCODE_F5);
        break;
    case ACTION_QUICK_LOAD:
        agent_simulate_key(SDL_SCANCODE_F7);
        break;
    }
}

} // namespace fallout
