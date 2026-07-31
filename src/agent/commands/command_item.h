#ifndef FALLOUT_AGENT_COMMAND_ITEM_H_
#define FALLOUT_AGENT_COMMAND_ITEM_H_

#include <cstdint>

namespace fallout {

enum ItemAction : uint8_t {
    ACTION_CHANGE_HAND = 0,
    ACTION_CHANGE_MODE = 1
};

void handle_command_item(unsigned char* msg);

} // namespace fallout

#endif /* FALLOUT_AGENT_COMMAND_ITEM_H_ */
