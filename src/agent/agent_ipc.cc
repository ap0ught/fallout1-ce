#include "agent/agent_ipc.h"
#include "agent/agent_commands.h"

#include <cstring>
#include <algorithm>
#include <SDL_net.h>

#include "game/gconfig.h"
#include "plib/gnw/memory.h"
#include "plib/gnw/debug.h"
#include "plib/gnw/input.h"
#include "plib/gnw/svga.h"

namespace fallout {

static SDLNet_SocketSet ICPSocketSet = nullptr;
static TCPsocket agentServer = nullptr;
static TCPsocket agentClient = nullptr;

bool agent_ipc_init()
{
    debug_printf(">agent_ipc_init\t");

    if (SDLNet_Init() < 0) {
        debug_printf("Failed to initialize SDL_net: %s\n", SDLNet_GetError());
        return false;
    }

    int port = -1;
    config_get_value(&game_config, GAME_CONFIG_AGENT_KEY, GAME_CONFIG_AGENT_PORT_KEY, &port);

    IPaddress ip;
    SDLNet_ResolveHost(&ip, nullptr, port);

    agentServer = SDLNet_TCP_Open(&ip);
    if (!agentServer) {
        debug_printf("Failed to open TCP server socket: %s\n", SDLNet_GetError());
        SDLNet_Quit();
        return false;
    }

    debug_printf("Listening for agent connection on port %d...\n", SDLNet_Read16(&ip.port));

    ICPSocketSet = SDLNet_AllocSocketSet(2);
    SDLNet_TCP_AddSocket(ICPSocketSet, agentServer);

    int timeout = -1;
    config_get_value(&game_config, GAME_CONFIG_AGENT_KEY, GAME_CONFIG_AGENT_START_TIMEOUT_KEY, &timeout);

    int ready = SDLNet_CheckSockets(ICPSocketSet, timeout);

    if (ready > 0) {
        if (SDLNet_SocketReady(agentServer)) {
            agentClient = SDLNet_TCP_Accept(agentServer);
            if (agentClient) {
                debug_printf("Agent process connected successfully.\n");
                SDLNet_TCP_AddSocket(ICPSocketSet, agentClient);

                agent_commands_init();
            } else {
                debug_printf("Failed to accept agent connection: %s\n", SDLNet_GetError());
                agent_ipc_shutdown();
                return false;
            }
        }
    } else {
        debug_printf("Agent process failed to connect within timeout period.\n");
        agent_ipc_shutdown();
        return false;
    }

    return true;
}

void agent_ipc_shutdown() 
{
    debug_printf(">agent_ipc_shutdown\t");

    if (agentClient) SDLNet_TCP_Close(agentClient);
    SDLNet_TCP_Close(agentServer);
    SDLNet_FreeSocketSet(ICPSocketSet);
    SDLNet_Quit();

    agent_commands_shutdown();

    ICPSocketSet = nullptr;
    agentServer = nullptr;
    agentClient = nullptr;

    return;
}

bool agent_ipc_send(uint8_t type, const unsigned char* data, uint32_t length)
{
    if (!agent_ipc_connected()) {
        return false;
    }

    unsigned char header[5];
    header[0] = type;
    SDLNet_Write32(length, header + 1);

    if (SDLNet_TCP_Send(agentClient, header, sizeof(header)) < (int)sizeof(header)) {
        debug_printf("Failed to send message header, disconnecting agent\n");
        agent_ipc_shutdown();
        return false;
    }

    uint32_t sent = 0;
    while (sent < length) {
        int to_send = std::min(static_cast<int>(length - sent), DATA_BUFFER_SIZE);
        int result = SDLNet_TCP_Send(agentClient, data + sent, to_send);
        if (result < to_send) {
            debug_printf("Failed to send message chunk at offset %u, disconnecting agent\n", sent);
            agent_ipc_shutdown();
            return false;
        }
        sent += to_send;
    }

    debug_printf("Sent message of %u byte(s) to agent\t", length);

    return true;
}

void agent_ipc_poll()
{
    if (!agent_ipc_connected()) {
        return;
    }

    agent_commands_update();

    int ready = SDLNet_CheckSockets(ICPSocketSet, 0);
    if (ready <= 0) {
        return;
    }

    unsigned char buf[DATA_BUFFER_SIZE];

    if (agentClient && SDLNet_SocketReady(agentClient)) {
        int recvLen = SDLNet_TCP_Recv(agentClient, buf, sizeof(buf) - 1);
        if (recvLen > 0) {
            buf[recvLen] = '\0';

            uint8_t msgType = buf[0];
            uint32_t msgLength = SDLNet_Read32(buf + 1);

            debug_printf("Received IPC message from agent: type 0x%02x, %u byte(s)\n", msgType, msgLength);

            agent_handle_command(msgType, msgLength, buf + 5);
        } else {
            debug_printf("Agent disconnected\n");
            agent_ipc_shutdown();
        }
    }
}

bool agent_ipc_connected() 
{
    return agentClient != nullptr;
}

} // namespace fallout
