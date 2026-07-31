#ifndef FALLOUT_AGENT_COMMAND_SAVE_H_
#define FALLOUT_AGENT_COMMAND_SAVE_H_

#include <cstdint>

namespace fallout {

enum SaveAction : uint8_t {
    ACTION_SAVE = 0,
    ACTION_LOAD = 1,
    ACTION_QUICK_SAVE = 2,
    ACTION_QUICK_LOAD = 3
};

void handle_command_save(unsigned char* msg, uint32_t length);

} // namespace fallout

#endif /* FALLOUT_AGENT_COMMAND_SAVE_H_ */
