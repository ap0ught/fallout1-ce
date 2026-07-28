#ifndef FALLOUT_GAME_AI_CONTROL_API_H_
#define FALLOUT_GAME_AI_CONTROL_API_H_

namespace fallout {

// Initialize the AI control API
void ai_control_api_init();

// Cleanup the AI control API
void ai_control_api_exit();

// Check if API is enabled
bool ai_control_api_enabled();

// Process one AI action from file and write current state
// Returns true if an action was processed
bool ai_control_api_process();

} // namespace fallout

#endif /* FALLOUT_GAME_AI_CONTROL_API_H_ */
