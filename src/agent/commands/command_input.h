#ifndef FALLOUT_AGENT_COMMAND_MOUSE_H_
#define FALLOUT_AGENT_COMMAND_MOUSE_H_

#include <cstdint>

#include "plib/gnw/dxinput.h"

namespace fallout {

enum MouseAction : uint8_t {
    ACTION_CLICK = 0,
    ACTION_CONTEXT_MENU = 1,
    ACTION_CONTEXT_SELECT = 2,
};

void handle_command_mouse(unsigned char* msg, MouseData* mouseData);

void handle_command_text_input(unsigned char* msg, uint32_t length);

} // namespace fallout

#endif /* FALLOUT_AGENT_COMMAND_MOUSE_H_ */
