#include "agent/agent_commands.h"
#include "agent/commands/command_combat.h"
#include "agent/commands/command_inventory.h"
#include "agent/commands/command_input.h"
#include "agent/commands/command_navigate.h"
#include "agent/commands/command_item.h"
#include "agent/commands/command_save.h"
#include "agent/agent_ipc.h"

#include <cstring>
#include <SDL.h>
#include <SDL_net.h>

#include "plib/gnw/memory.h"
#include "plib/gnw/debug.h"
#include "plib/gnw/svga.h"
#include "plib/gnw/input.h"

#include "game/object.h"
#include "game/combat.h"
#include "game/stat.h"

namespace fallout {

static int screenshotSize;
static unsigned char* screenshotBuf = nullptr;
static MouseData agentMouseData;

static bool agentPendingResponse = false;
int gAgentCommandsUpdateDelay = 0;

bool gAgentLShiftHeld = false;

void agent_commands_init()
{
    screenshotSize = (sizeof(uint32_t) * 2) + (screenGetWidth() * screenGetHeight() * 4);
    screenshotBuf = (unsigned char*)mem_malloc(screenshotSize);

    agentMouseData.x = -1;
    agentMouseData.y = -1;
    agentMouseData.buttons[0] = 0;
    agentMouseData.buttons[1] = 0;
}

void agent_commands_shutdown()
{
    if (screenshotBuf) {
        mem_free(screenshotBuf);
        screenshotBuf = nullptr;
    }
}

bool agent_response_is_pending()
{
    return agentPendingResponse;
}

bool agent_get_mouse_state(MouseData* mouseData)
{
    if (!agent_ipc_connected()) {
        return false;
    }

    if (agentMouseData.x == -1 && agentMouseData.y == -1) {
        return false;
    }

    memcpy(mouseData, &agentMouseData, sizeof(MouseData));

    return true;
}

void agent_handle_command(uint8_t command, uint32_t length, unsigned char* msg)
{
    agentPendingResponse = true;

    switch (command) {
    case MSG_NAVIGATE:
        handle_command_navigate(msg, &agentMouseData);
        break;
    case MSG_COMBAT:
        handle_command_combat(msg, &agentMouseData);
        break;
    case MSG_INVENTORY:
        handle_command_inventory(msg);
        break;
    case MSG_LOOT:
        handle_command_loot(msg);
        break;
    case MSG_BARTER:
        handle_command_barter(msg);
        break;
    case MSG_ITEM:
        handle_command_item(msg);
        break;
    case MSG_SAVE:
        handle_command_save(msg, length);
        break;
    case MSG_MOUSE:
        handle_command_mouse(msg, &agentMouseData);
        break;
    case MSG_TEXT_INPUT:
        handle_command_text_input(msg, length);
        break;
    case MSG_REQ_STATE:
        break;
    default:
        debug_printf("Unknown agent command type: 0x%02x\n", command);
        break;
    }
}

void agent_send_text(const char* text)
{
    if (!agent_ipc_connected() || text == nullptr) {
        return;
    }

    agent_ipc_send(MSG_GENERAL,
        reinterpret_cast<const unsigned char*>(text),
        static_cast<uint32_t>(strlen(text)));
}

static bool agent_send_player_state()
{
    unsigned char buf[24];
    int hp = stat_level(obj_dude, STAT_CURRENT_HIT_POINTS);
    int maxHp = stat_level(obj_dude, STAT_MAXIMUM_HIT_POINTS);
    int ap = obj_dude->data.critter.combat.ap;
    int maxAp = stat_level(obj_dude, STAT_MAXIMUM_ACTION_POINTS);
    int ac = stat_level(obj_dude, STAT_ARMOR_CLASS);
    int level = stat_pc_get(PC_STAT_LEVEL);

    SDLNet_Write32(hp, buf);
    SDLNet_Write32(maxHp, buf + 4);
    SDLNet_Write32(ap, buf + 8);
    SDLNet_Write32(maxAp, buf + 12);
    SDLNet_Write32(ac, buf + 16);
    SDLNet_Write32(level, buf + 20);

    return agent_ipc_send(MSG_PLAYER_STATE, buf, sizeof(buf));
}

static bool agent_send_screenshot()
{
    if (gSdlRenderer == nullptr) {
        debug_printf("No renderer available for sending screenshot\n");
        return false;
    };

    if (SDL_RenderReadPixels(
            gSdlRenderer,
            NULL,
            SDL_PIXELFORMAT_RGB888,
            screenshotBuf + (sizeof(uint32_t) * 2),
            gSdlTextureSurface->pitch)
        < 0) {

        debug_printf("Failed to read pixels from renderer: %s\n", SDL_GetError());
        return false;
    }

    SDLNet_Write32(screenGetWidth(), screenshotBuf);
    SDLNet_Write32(screenGetHeight(), screenshotBuf + sizeof(uint32_t));

    return agent_ipc_send(MSG_SCREENSHOT, screenshotBuf, screenshotSize);
}

void agent_commands_update()
{
    agentMouseData.x = -1;
    agentMouseData.y = -1;
    agentMouseData.buttons[0] = 0;
    agentMouseData.buttons[1] = 0;

    if (gAgentCommandsUpdateDelay > 0) {
        gAgentCommandsUpdateDelay--;
        return;
    }

    if (gAgentLShiftHeld) {
        KeyboardData keyboardData;
        keyboardData.key = SDL_SCANCODE_LSHIFT;
        keyboardData.down = false;
        agent_process_key(&keyboardData);

        gAgentLShiftHeld = false;
    }

    if (!agentPendingResponse) return;

    // Wait until the player's turn to send a response
    if ((isInCombat() && combat_whose_turn() != obj_dude) || anim_busy(obj_dude) != 0) {
        return;
    }

    agent_send_screenshot();
    agent_send_player_state();
    agent_ipc_send(MSG_RESPONSE_END, nullptr, 0);

    agentPendingResponse = false;
}

} // namespace fallout
