#include "command_input.h"
#include "agent/agent_commands.h"

#include <cstdint>
#include <SDL.h>
#include <SDL_net.h>

#include "game/gmouse.h"
#include "game/map.h"
#include "plib/gnw/button.h"
#include "plib/gnw/debug.h"
#include "plib/gnw/input.h"
#include "plib/gnw/mouse.h"
#include "plib/gnw/svga.h"

namespace fallout {

static SDL_Scancode mapCharToScancode(char ch)
{
    if (ch >= 'a' && ch <= 'z')
        return static_cast<SDL_Scancode>(SDL_SCANCODE_A + (ch - 'a'));
    if (ch >= 'A' && ch <= 'Z')
        return static_cast<SDL_Scancode>(SDL_SCANCODE_A + (ch - 'A'));
    if (ch >= '1' && ch <= '9')
        return static_cast<SDL_Scancode>(SDL_SCANCODE_1 + (ch - '1'));
    if (ch == '0')
        return SDL_SCANCODE_0;

    switch (ch) {
    case ' ':
        return SDL_SCANCODE_SPACE;
    case '-':
        return SDL_SCANCODE_MINUS;
    case '=':
        return SDL_SCANCODE_EQUALS;
    case '[':
        return SDL_SCANCODE_LEFTBRACKET;
    case ']':
        return SDL_SCANCODE_RIGHTBRACKET;
    case '\\':
        return SDL_SCANCODE_BACKSLASH;
    case ';':
        return SDL_SCANCODE_SEMICOLON;
    case '\'':
        return SDL_SCANCODE_APOSTROPHE;
    case ',':
        return SDL_SCANCODE_COMMA;
    case '.':
        return SDL_SCANCODE_PERIOD;
    case '/':
        return SDL_SCANCODE_SLASH;
    case '`':
        return SDL_SCANCODE_GRAVE;
    case '\t':
        return SDL_SCANCODE_TAB;
    case '\n':
    case '\r':
        return SDL_SCANCODE_RETURN;
    default:
        return SDL_SCANCODE_UNKNOWN;
    }
}

void handle_command_text_input(unsigned char* msg, uint32_t length)
{
    uint32_t textLength = SDLNet_Read32(msg);
    const char* text = reinterpret_cast<const char*>(msg + 4);

    for (uint32_t i = 0; i < textLength; i++) {
        char ch = text[i];
        SDL_Scancode sc = mapCharToScancode(ch);

        if (sc == SDL_SCANCODE_UNKNOWN) {
            debug_printf("Unknown scancode for character: %c\n", ch);
            continue;
        }

        bool uppercase = (ch >= 'A' && ch <= 'Z');
        agent_simulate_key(sc, uppercase);
    }
}

void handle_command_mouse(unsigned char* msg, MouseData* mouseData)
{
    uint8_t action = msg[0];
    uint32_t x = SDLNet_Read32(msg + 1);
    uint32_t y = SDLNet_Read32(msg + 5);

    if (x >= static_cast<uint32_t>(screenGetWidth()) || y >= static_cast<uint32_t>(screenGetHeight())) {
        x = static_cast<uint32_t>(screenGetWidth() / 2);
        y = static_cast<uint32_t>(screenGetHeight() / 2);
    }

    switch (action) {
    case ACTION_CONTEXT_SELECT:
        gmouse_3d_select_context_menu_item(x);
        break;
    case ACTION_CONTEXT_MENU:
        gmouse_3d_set_mode(GAME_MOUSE_MODE_ARROW);
        mouse_set_position(x, y);
        gmouse_3d_open_context_menu(x, y, map_elevation);
        break;
    case ACTION_CLICK:
        gmouse_3d_set_mode(GAME_MOUSE_MODE_ARROW);
        GNW_reset_last_button_winID();
        mouseData->x = x;
        mouseData->y = y;
        mouseData->buttons[0] = 1;
        mouseData->buttons[1] = 0;
        mouse_get_agent_delta(mouseData);
        break;
    }

    gAgentCommandsUpdateDelay = 3;
}

} // namespace fallout
