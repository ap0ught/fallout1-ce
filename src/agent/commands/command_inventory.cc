#include "agent/agent_commands.h"
#include "command_inventory.h"

#include <SDL_net.h>
#include <cstdint>

#include "game/inventry.h"
#include "game/gmouse.h"
#include "plib/gnw/dxinput.h"
#include "plib/gnw/input.h"

namespace fallout {

void handle_command_inventory(unsigned char* msg)
{
    uint8_t action = msg[0];
    if (action == ACTION_INVENTORY_OPEN) {
        agent_simulate_key(SDL_SCANCODE_I);
    } else if (action == ACTION_INVENTORY_EQUIP) {
        uint32_t item = SDLNet_Read32(msg + 1);
        uint8_t hand = msg[5];
        agent_equip_item(item, hand);
    } else if (action == ACTION_INVENTORY_UNEQUIP) {
        uint8_t slot = msg[1];
        agent_unequip_item(slot);
    }
}

void handle_command_loot(unsigned char* msg) {
    uint8_t take = msg[0];
    uint8_t item = msg[1];
    uint32_t quantity = SDLNet_Read32(msg + 2);
    agent_loot_item(item, take, quantity);
}

void handle_command_barter(unsigned char* msg)
{
    uint8_t action = msg[0];
    uint32_t item = SDLNet_Read32(msg + 1);
    uint32_t quantity = SDLNet_Read32(msg + 5);

    if (agent_barter_item(1, action, 1) == -1) {

    }
}

} // namespace fallout
