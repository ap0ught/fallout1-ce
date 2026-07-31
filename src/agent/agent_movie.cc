#include "agent/agent_movie.h"
#include "agent/agent_commands.h"

#include "game/gmovie.h"

namespace fallout {

// clang-format off
static const char* movie_summaries[MOVIE_COUNT] = {
    nullptr, // MOVIE_IPLOGO
    nullptr, // MOVIE_MPLOGO
    "Intro cinematic: War never changes. The year is 2161. You are the Vault Dweller "
    "from Vault 13. The vault's water purification chip has broken, and you have been "
    "chosen to venture into the wasteland to find a replacement.",
    "Cinematic: The Mariposa Military Base has exploded. The vats of FEV (Forced "
    "Evolutionary Virus) have been destroyed, dealing a major blow to the Super Mutant army.",
    "Cinematic: The Cathedral, the Master's base of operations, has been destroyed "
    "in a massive explosion.",
    "Overseer briefing: The Overseer explains that Vault 13's water recycling chip "
    "has failed. You must find a replacement water chip or everyone in the vault will "
    "die. He marks Vault 15 on your map as a starting point. You have 150 days.",
    "Death cinematic: The vault's water supply has run out. The residents of Vault 13 "
    "have perished from dehydration. You have failed your mission. Game over.",
    nullptr, // MOVIE_OVRRUN
    nullptr, // MOVIE_WALKM (endgame.cc sends custom text)
    nullptr, // MOVIE_WALKW (endgame.cc sends custom text)
    nullptr, // MOVIE_DIPEDV
    nullptr, // MOVIE_BOIL1
    nullptr, // MOVIE_BOIL2
    nullptr, // MOVIE_RAEKILLS
};
// clang-format on

void agent_send_movie_summary(int game_movie)
{
    if (game_movie < 0 || game_movie >= MOVIE_COUNT) {
        return;
    }

    const char* summary = movie_summaries[game_movie];
    if (summary != nullptr) {
        agent_send_text(summary);
    }
}

} // namespace fallout
