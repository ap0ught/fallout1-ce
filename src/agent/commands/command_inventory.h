#ifndef FALLOUT_AGENT_COMMAND_INVENTORY_H_
#define FALLOUT_AGENT_COMMAND_INVENTORY_H_

#include <cstdint>

#include "plib/gnw/dxinput.h"

namespace fallout {

enum InventoryAction : uint8_t {
    ACTION_INVENTORY_OPEN = 0,
    ACTION_INVENTORY_EQUIP = 1,
    ACTION_INVENTORY_UNEQUIP = 2,
};

enum BarterAction : uint8_t {
    ACTION_BUY = 0,
    ACTION_SELL = 1,
    ACTION_RETRACT_MINE = 2,
    ACTION_RETRACT_THEIRS = 3,
    ACTION_BARTER_CONFIRM = 4
};

void handle_command_inventory(unsigned char* msg);
void handle_command_loot(unsigned char* msg);
void handle_command_barter(unsigned char* msg);

} // namespace fallout

#endif /* FALLOUT_AGENT_COMMAND_INVENTORY_H_ */
