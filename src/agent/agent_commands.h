#ifndef FALLOUT_AGENT_COMMANDS_H_
#define FALLOUT_AGENT_COMMANDS_H_

#include <cstdint>

#include "game/object_types.h"
#include "plib/gnw/dxinput.h"

namespace fallout {

extern int gAgentCommandsUpdateDelay;
extern bool gAgentLShiftHeld;

void agent_commands_init();

void agent_commands_shutdown();

void agent_handle_command(uint8_t command, uint32_t length, unsigned char* msg);

void agent_commands_update();

bool agent_response_is_pending();

bool agent_get_mouse_state(MouseData* mouseData);

void agent_send_text(const char* text);

} // namespace fallout


#endif /* FALLOUT_AGENT_COMMANDS_H_ */
