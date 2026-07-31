#ifndef FALLOUT_AGENT_COMMAND_NAVIGATE_H_
#define FALLOUT_AGENT_COMMAND_NAVIGATE_H_

#include <cstdint>

#include "plib/gnw/dxinput.h"

namespace fallout {

enum NavigateAction : uint8_t {
    ACTION_WALK = 0,
    ACTION_RUN = 1,
    ACTION_SCROLL_LEFT = 2,
    ACTION_SCROLL_RIGHT = 3,
    ACTION_SCROLL_UP = 4,
    ACTION_SCROLL_DOWN = 5,
    ACTION_CENTER = 6
};

void handle_command_navigate(unsigned char* msg, MouseData* mouseData);

} // namespace fallout

#endif /* FALLOUT_AGENT_COMMAND_NAVIGATE_H_ */
