#ifndef FALLOUT_AGENT_COMMAND_COMBAT_H_
#define FALLOUT_AGENT_COMMAND_COMBAT_H_

#include <cstdint>

#include "plib/gnw/dxinput.h"

namespace fallout {

enum CombatAction : uint8_t {
    ACTION_BEGIN = 0,
    ACTION_END_TURN = 1,
    ACTION_END_COMBAT = 2,
    ACTION_ATTACK = 3
};

void handle_command_combat(unsigned char* msg, MouseData* mouseData);

} // namespace fallout

#endif /* FALLOUT_AGENT_COMMAND_COMBAT_H_ */
