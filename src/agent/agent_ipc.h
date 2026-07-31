#ifndef FALLOUT_AGENT_IPC_H_
#define FALLOUT_AGENT_IPC_H_

#include <cstdint>

namespace fallout {

#define DATA_BUFFER_SIZE 1024

enum AgentMessageType : uint8_t {
    MSG_GENERAL = 0x00,
    MSG_SCREENSHOT = 0x01,
    MSG_PLAYER_STATE = 0x02,
    MSG_RESPONSE_END = 0x03,

    MSG_NAVIGATE = 0x10,
    MSG_COMBAT = 0x11,
    MSG_INVENTORY = 0x12,
    MSG_BARTER = 0x13,
    MSG_ITEM = 0x14,
    MSG_LOOT = 0x15,
    MSG_REQ_STATE = 0x16,
    MSG_SAVE = 0x17,
    MSG_MOUSE = 0x18,
    MSG_TEXT_INPUT = 0x19,
};

// Initialize the agent IPC: starts a TCP server on localhost and waits up
// to 10 seconds (set in config) for the Python agent to connect.
// Returns true on success. Logs warnings and returns false on failure (the
// game continues normally without agent functionality).
bool agent_ipc_init();

// Close the client/server sockets.
void agent_ipc_shutdown();

// Check (non-blocking) whether the Python agent has sent a request.
// Registered as a background process so it runs from get_input().
void agent_ipc_poll();

// Returns true if the Python agent is currently connected.
bool agent_ipc_connected();

// Send a message: 5-byte header (1 byte type + 4 bytes length)
// followed by the payload in 1024-byte chunks.
bool agent_ipc_send(uint8_t type, const unsigned char* data, uint32_t length);

} // namespace fallout

#endif /* FALLOUT_AGENT_IPC_H_ */
