#include "game/ai_control_api.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "game/actions.h"
#include "game/combat.h"
#include "game/critter.h"
#include "game/game.h"
#include "game/gconfig.h"
#include "game/inventry.h"
#include "game/item.h"
#include "game/map.h"
#include "game/object.h"
#include "game/perk.h"
#include "game/protinst.h"
#include "game/skill.h"
#include "game/stat.h"
#include "game/tile.h"
#include "game/trait.h"
#include "game/worldmap.h"

namespace fallout {

// Configuration
static bool gAiControlApiEnabled = false;
static const char* kActionFilePath = "ai_action.json";
static const char* kStateFilePath = "ai_state.json";
static const char* kEventsFilePath = "ai_events.json";
static const char* kKnowledgeFilePath = "ai_knowledge.json";
static const char* kMemoryFilePath = "ai_memory.json";

// Event tracking
#define MAX_RECENT_EVENTS 50
static char gRecentEvents[MAX_RECENT_EVENTS][256];
static int gEventCount = 0;
static int gEventWriteIndex = 0;

// Action tracking
static char gLastActionResult[256] = "none";
static int gLastActionTimestamp = 0;
static int gActionCooldownMs = 100; // 100ms cooldown between actions

// Stats tracking for Twitch overlays
static int gTotalDamageDealt = 0;
static int gTotalKills = 0;
static int gSessionStartTime = 0;
static int gLastHitPoints = 0;
static int gLastLevel = 0;

// Knowledge base written once at initialization
static bool gKnowledgeWritten = false;

// Memory tracking
#define MAX_MEMORY_ENTRIES 200
struct MemoryEntry {
    int tile;
    int elevation;
    char mapName[32];
    char action[64];
    char target[64];
    char result[128];
    int timestamp;
    bool active;
};
static MemoryEntry gMemoryEntries[MAX_MEMORY_ENTRIES];
static int gMemoryIndex = 0;
static int gMemoryCount = 0;

// Items collected tracking
#define MAX_ITEMS_COLLECTED 500
struct ItemCollected {
    int pid;
    char name[64];
    int quantity;
    char mapName[32];
    int timestamp;
    bool active;
};
static ItemCollected gItemsCollected[MAX_ITEMS_COLLECTED];
static int gItemsCollectedIndex = 0;
static int gItemsCollectedCount = 0;

// Quest/milestone tracking
#define MAX_MILESTONES 100
struct Milestone {
    char description[128];
    char location[32];
    int timestamp;
    bool active;
};
static Milestone gMilestones[MAX_MILESTONES];
static int gMilestonesIndex = 0;
static int gMilestonesCount = 0;

// Simple JSON writer helpers
class JsonWriter {
public:
    JsonWriter() : buffer_pos(0), first_item(true) {}
    
    void startObject() {
        append("{");
        first_item = true;
    }
    
    void endObject() {
        append("}");
        first_item = false;
    }
    
    void startArray(const char* name) {
        if (!first_item) append(",");
        append("\"");
        append(name);
        append("\":[");
        first_item = true;
    }
    
    void endArray() {
        append("]");
        first_item = false;
    }
    
    void addString(const char* name, const char* value) {
        if (!first_item) append(",");
        append("\"");
        append(name);
        append("\":\"");
        if (value) append(value);
        append("\"");
        first_item = false;
    }
    
    void addInt(const char* name, int value) {
        if (!first_item) append(",");
        char temp[64];
        snprintf(temp, sizeof(temp), "\"%s\":%d", name, value);
        append(temp);
        first_item = false;
    }
    
    void addBool(const char* name, bool value) {
        if (!first_item) append(",");
        append("\"");
        append(name);
        append(value ? "\":true" : "\":false");
        first_item = false;
    }
    
    void addFloat(const char* name, float value) {
        if (!first_item) append(",");
        char temp[64];
        snprintf(temp, sizeof(temp), "\"%s\":%.2f", name, value);
        append(temp);
        first_item = false;
    }
    
    void addObjectInArray() {
        if (!first_item) append(",");
        append("{");
        first_item = true;
    }
    
    void endObjectInArray() {
        append("}");
        first_item = false;
    }
    
    const char* c_str() const { return buffer; }
    size_t length() const { return buffer_pos; }
    
private:
    static const size_t BUFFER_SIZE = 65536; // 64KB buffer
    char buffer[BUFFER_SIZE];
    size_t buffer_pos;
    bool first_item;
    
    void append(const char* str) {
        size_t len = strlen(str);
        if (buffer_pos + len < BUFFER_SIZE - 1) {
            strcpy(buffer + buffer_pos, str);
            buffer_pos += len;
        }
    }
};

// Helper to add an event to the log
static void addEvent(const char* eventType, const char* description) {
    if (gEventCount >= MAX_RECENT_EVENTS * 10) {
        // Reset if we've wrapped too many times
        gEventCount = 0;
        gEventWriteIndex = 0;
    }
    
    snprintf(gRecentEvents[gEventWriteIndex], sizeof(gRecentEvents[gEventWriteIndex]),
             "%s: %s", eventType, description);
    
    gEventWriteIndex = (gEventWriteIndex + 1) % MAX_RECENT_EVENTS;
    gEventCount++;
}

// Get current timestamp in milliseconds
static int getCurrentTimeMs() {
    return (int)(time(NULL) * 1000);
}

// Add entry to memory log
static void addMemory(const char* action, const char* target, const char* result) {
    if (!obj_dude) return;
    
    MemoryEntry* entry = &gMemoryEntries[gMemoryIndex];
    entry->tile = obj_dude->tile;
    entry->elevation = obj_dude->elevation;
    
    int mapIndex = map_get_index_number();
    if (mapIndex != -1) {
        char* mapName = map_get_short_name(mapIndex);
        if (mapName) {
            strncpy(entry->mapName, mapName, sizeof(entry->mapName) - 1);
            entry->mapName[sizeof(entry->mapName) - 1] = '\0';
        }
    }
    
    strncpy(entry->action, action, sizeof(entry->action) - 1);
    entry->action[sizeof(entry->action) - 1] = '\0';
    
    strncpy(entry->target, target, sizeof(entry->target) - 1);
    entry->target[sizeof(entry->target) - 1] = '\0';
    
    strncpy(entry->result, result, sizeof(entry->result) - 1);
    entry->result[sizeof(entry->result) - 1] = '\0';
    
    entry->timestamp = getCurrentTimeMs() / 1000; // seconds since epoch
    entry->active = true;
    
    gMemoryIndex = (gMemoryIndex + 1) % MAX_MEMORY_ENTRIES;
    if (gMemoryCount < MAX_MEMORY_ENTRIES) {
        gMemoryCount++;
    }
}

// Track item collected
static void addItemCollected(int pid, const char* itemName, int quantity) {
    if (!obj_dude) return;
    
    ItemCollected* item = &gItemsCollected[gItemsCollectedIndex];
    item->pid = pid;
    item->quantity = quantity;
    
    strncpy(item->name, itemName ? itemName : "Unknown", sizeof(item->name) - 1);
    item->name[sizeof(item->name) - 1] = '\0';
    
    int mapIndex = map_get_index_number();
    if (mapIndex != -1) {
        char* mapName = map_get_short_name(mapIndex);
        if (mapName) {
            strncpy(item->mapName, mapName, sizeof(item->mapName) - 1);
            item->mapName[sizeof(item->mapName) - 1] = '\0';
        }
    }
    
    item->timestamp = getCurrentTimeMs() / 1000;
    item->active = true;
    
    gItemsCollectedIndex = (gItemsCollectedIndex + 1) % MAX_ITEMS_COLLECTED;
    if (gItemsCollectedCount < MAX_ITEMS_COLLECTED) {
        gItemsCollectedCount++;
    }
}

// Track milestone
static void addMilestone(const char* description) {
    if (!obj_dude) return;
    
    Milestone* milestone = &gMilestones[gMilestonesIndex];
    
    strncpy(milestone->description, description, sizeof(milestone->description) - 1);
    milestone->description[sizeof(milestone->description) - 1] = '\0';
    
    int mapIndex = map_get_index_number();
    if (mapIndex != -1) {
        char* mapName = map_get_short_name(mapIndex);
        if (mapName) {
            strncpy(milestone->location, mapName, sizeof(milestone->location) - 1);
            milestone->location[sizeof(milestone->location) - 1] = '\0';
        }
    }
    
    milestone->timestamp = getCurrentTimeMs() / 1000;
    milestone->active = true;
    
    gMilestonesIndex = (gMilestonesIndex + 1) % MAX_MILESTONES;
    if (gMilestonesCount < MAX_MILESTONES) {
        gMilestonesCount++;
    }
}

// Write comprehensive character data for external website
static void writeMemory() {
    JsonWriter json;
    json.startObject();
    
    json.addString("description", "AI decision memory - records actions, outcomes, and learned experiences");
    json.addInt("total_memories", gMemoryCount);
    
    json.startArray("memories");
    int startIdx = (gMemoryCount >= MAX_MEMORY_ENTRIES) ? gMemoryIndex : 0;
    for (int i = 0; i < gMemoryCount && i < MAX_MEMORY_ENTRIES; i++) {
        int idx = (startIdx + i) % MAX_MEMORY_ENTRIES;
        if (gMemoryEntries[idx].active) {
            json.addObjectInArray();
            json.addString("map", gMemoryEntries[idx].mapName);
            json.addInt("tile", gMemoryEntries[idx].tile);
            json.addInt("elevation", gMemoryEntries[idx].elevation);
            json.addString("action", gMemoryEntries[idx].action);
            json.addString("target", gMemoryEntries[idx].target);
            json.addString("result", gMemoryEntries[idx].result);
            json.addInt("timestamp", gMemoryEntries[idx].timestamp);
            json.endObjectInArray();
        }
    }
    json.endArray();
    
    json.endObject();
    
    char tempPath[256];
    snprintf(tempPath, sizeof(tempPath), "%s.tmp", kMemoryFilePath);
    
    FILE* fp = fopen(tempPath, "w");
    if (fp) {
        fwrite(json.c_str(), 1, json.length(), fp);
        fclose(fp);
        rename(tempPath, kMemoryFilePath);
    }
}

// Write game knowledge base (written once at initialization)
static void writeGameKnowledge() {
    if (gKnowledgeWritten) {
        return;
    }
    
    JsonWriter json;
    json.startObject();
    
    // Character/Roleplay Context - KEEP IN CHARACTER
    json.addString("character_role", "Vault Dweller from Vault 13");
    json.addString("character_background", "You are a resident of Vault 13, an underground shelter built before the nuclear war. Your home is running out of water due to a broken water purification chip. The Overseer has chosen you to venture into the dangerous wasteland to find a replacement chip. You have 150 days before the vault runs out of water. You are brave but inexperienced in the harsh realities of the post-apocalyptic world.");
    json.addString("roleplay_guidelines", "Stay in character as the Vault Dweller. You're cautious but determined. You care about your vault's survival. You're unfamiliar with the wasteland initially but learn quickly. Speak in first person when describing actions. Show concern for survival (HP, resources). Be wary of strangers but willing to help good people. Your mission is urgent but you must survive to complete it.");
    json.addString("character_motivation", "Primary: Find the water chip to save Vault 13. Secondary: Survive the wasteland, help innocents, stop threats to humanity.");
    json.addString("speaking_style", "Practical and straightforward. Example: 'I need to find that water chip, but I should heal first - I'm badly injured.' or 'There's a hostile creature ahead. I'll need to fight or find another way around.'");
    
    // Game overview
    json.addString("game_title", "Fallout 1");
    json.addString("genre", "Post-apocalyptic RPG");
    json.addString("setting", "Post-nuclear war wasteland, Southern California, year 2161 (84 years after the bombs fell in 2077)");
    json.addString("world_state", "Civilization destroyed by nuclear war. Survivors live in vaults, settlements, or as raiders. Mutated creatures roam the wastes. Technology is scarce and valuable. Water and food are precious. Violence is common. Some areas are irradiated.");
    
    // Core gameplay loop
    json.startArray("core_objectives");
    json.addObjectInArray();
    json.addString("objective", "Find water chip for Vault 13");
    json.addString("time_limit", "150 days initially (can be extended)");
    json.addString("urgency", "CRITICAL - Your vault will die without water");
    json.endObjectInArray();
    json.addObjectInArray();
    json.addString("objective", "Investigate the Master and Super Mutant army");
    json.addString("discovery", "Later in game: Stop the Master's plan to convert humanity into Super Mutants");
    json.endObjectInArray();
    json.endArray();
    
    // Available actions
    json.startArray("available_actions");
    const char* actions[][2] = {
        {"move", "Move player to target tile. Use for exploration and positioning. Costs AP in combat."},
        {"attack", "Attack a target at specified tile. Requires weapon and line of sight. Costs AP."},
        {"use_item", "Use an item from inventory by PID. Includes Stimpaks for healing, tools, consumables."},
        {"pickup", "Pick up item at target tile. Used to collect loot, ammunition, and quest items."},
        {"wait", "Skip turn in combat or wait. Ends your turn and gives enemies their turn."}
    };
    for (int i = 0; i < 5; i++) {
        json.addObjectInArray();
        json.addString("action", actions[i][0]);
        json.addString("description", actions[i][1]);
        json.endObjectInArray();
    }
    json.endArray();
    
    // Detailed weapon database
    json.startArray("weapons_database");
    const char* weapons[][6] = {
        // Name, Damage, Range, AP Cost, Ammo Type, Notes
        {"Knife", "1-6", "1", "3", "None", "Starting melee weapon. Weak but no ammo needed."},
        {"Spear", "3-10", "2", "4", "None", "Good early melee. Can be thrown."},
        {"10mm Pistol", "5-12", "20", "5", "10mm", "Common early gun. Accurate, low damage."},
        {"Desert Eagle", "10-16", "25", "5", ".44", "Powerful pistol. Good damage, rare ammo."},
        {"Shotgun", "12-22", "14", "5", "12 gauge", "High damage, short range. Excellent vs unarmored."},
        {"Hunting Rifle", "8-20", "40", "5", ".223", "Long range sniper. High accuracy."},
        {"Assault Rifle", "8-16", "45", "5", "5mm", "Burst fire. Good all-around weapon."},
        {"SMG", "5-12", "32", "4", "10mm", "Burst fire. High AP cost but many shots."},
        {"Combat Shotgun", "15-25", "22", "5", "12 gauge", "Upgraded shotgun. Devastating close range."},
        {"Laser Pistol", "10-22", "35", "5", "Energy cell", "Energy weapon. Good vs armor."},
        {"Plasma Rifle", "30-65", "25", "5", "Microfusion", "Late game. Extremely powerful."},
        {"Rocket Launcher", "35-100", "40", "6", "Rocket", "Explosive. Area damage. Very rare ammo."},
        {"Minigun", "7-11", "35", "6", "5mm", "Burst fire. Shreds targets with many bullets."},
        {"Turbo Plasma Rifle", "35-70", "30", "4", "Microfusion", "Best energy weapon. Fast and deadly."}
    };
    for (int i = 0; i < 14; i++) {
        json.addObjectInArray();
        json.addString("name", weapons[i][0]);
        json.addString("damage", weapons[i][1]);
        json.addString("range", weapons[i][2]);
        json.addString("ap_cost", weapons[i][3]);
        json.addString("ammo", weapons[i][4]);
        json.addString("notes", weapons[i][5]);
        json.endObjectInArray();
    }
    json.endArray();
    
    // Armor database
    json.startArray("armor_database");
    const char* armor[][4] = {
        // Name, AC Bonus, Damage Resist, Notes
        {"None", "0", "0%", "No protection. Very vulnerable."},
        {"Leather Jacket", "8", "20%", "Basic early armor. Light protection."},
        {"Leather Armor", "15", "25%", "Better leather. Decent early game."},
        {"Metal Armor", "10", "30%", "Heavy but good protection. Slows movement."},
        {"Tesla Armor", "15", "20% (80% vs energy)", "Specialized. Excellent vs energy weapons."},
        {"Combat Armor", "20", "40%", "Military grade. Strong all-around protection."},
        {"Power Armor", "25", "40%", "Best armor. +3 Strength. Rare. Quest reward."},
        {"Hardened Power Armor", "30", "50%", "Upgraded power armor. Ultimate protection."}
    };
    for (int i = 0; i < 8; i++) {
        json.addObjectInArray();
        json.addString("name", armor[i][0]);
        json.addString("ac_bonus", armor[i][1]);
        json.addString("damage_resist", armor[i][2]);
        json.addString("notes", armor[i][3]);
        json.endObjectInArray();
    }
    json.endArray();
    
    // Enemy database with weaknesses
    json.startArray("enemy_database");
    const char* enemies[][5] = {
        // Name, HP Range, Weakness, Strength, Strategy
        {"Rat", "5-15", "Any weapon", "Fast, numerous", "Easy kills. Save ammo, use melee."},
        {"Radscorpion", "20-40", "Energy weapons, eyes", "Poison tail, armor", "Aim for eyes. Avoid poison. Use ranged."},
        {"Raider", "30-60", "Headshots, better gear", "Numbers, guns", "Use cover. Aim for head. Loot their weapons."},
        {"Super Mutant", "80-140", "Plasma/energy, eyes", "High HP, strong weapons", "DANGEROUS. Use best weapons. Aim for eyes/head."},
        {"Deathclaw", "200-300", "Eye shots, plasma", "Extreme damage, fast", "DEADLY. Run if possible. Plasma rifle to eyes only."},
        {"Ghoul", "40-70", "Fire, headshots", "Radiation immune", "Use fire weapons or target head. Not all hostile."},
        {"Centaur", "90-120", "Energy weapons", "Multiple attacks, tough", "Mutant creature. Use plasma or rockets."},
        {"Floater", "40-80", "Energy/explosive", "Ranged acid", "Keep distance. Use grenades or energy weapons."},
        {"Robot", "50-150", "Pulse/EMP, rockets", "Armor, sensors", "EMP weapons best. Explosives good. Lasers weak."},
        {"Mutated Animals", "15-50", "Any weapons", "Speed, surprise", "Mantis, wild dogs. Moderate threat."}
    };
    for (int i = 0; i < 10; i++) {
        json.addObjectInArray();
        json.addString("enemy", enemies[i][0]);
        json.addString("hp_range", enemies[i][1]);
        json.addString("weakness", enemies[i][2]);
        json.addString("strength", enemies[i][3]);
        json.addString("combat_strategy", enemies[i][4]);
        json.endObjectInArray();
    }
    json.endArray();
    
    // Important item PIDs with detailed stats
    json.startArray("common_items");
    const char* items[][3] = {
        {"40", "Stimpak", "Heals 15-20 HP. Essential for survival. Use when HP is low."},
        {"41", "Caps ($)", "Currency. Used for trading and bartering. Collect from containers and enemies."},
        {"144", "Super Stimpak", "Heals more HP than regular Stimpak. Rare and valuable."},
        {"47", "First Aid Kit", "Used with First Aid skill to heal. More effective with higher skill."},
        {"91", "Doctor's Bag", "Used with Doctor skill to heal critical injuries."},
        {"48", "RadAway", "Reduces radiation. Important in irradiated areas."},
        {"52", "Geiger Counter", "Measures radiation levels. Helps avoid dangerous areas."}
    };
    for (int i = 0; i < 7; i++) {
        json.addObjectInArray();
        json.addString("pid", items[i][0]);
        json.addString("name", items[i][1]);
        json.addString("usage", items[i][2]);
        json.endObjectInArray();
    }
    json.endArray();
    
    // Ammunition types
    json.startArray("ammunition_types");
    const char* ammo[][3] = {
        {"10mm", "Common early game", "Used by 10mm Pistol, SMG. Widely available."},
        {".44 Magnum", "Powerful pistol rounds", "Desert Eagle ammo. Good damage, less common."},
        {"12 gauge", "Shotgun shells", "Devastating close range. Watch your stock."},
        {".223 FMJ", "Rifle ammunition", "Hunting/Assault Rifle. Medium availability."},
        {"5mm", "Minigun/Assault ammo", "Heavy use in auto weapons. Stock up."},
        {"Small Energy Cell", "Energy weapon ammo", "Laser weapons. Scarce early, common late."},
        {"Microfusion Cell", "Plasma weapon ammo", "Most powerful. Very rare. Don't waste."},
        {"Rocket", "Explosive", "Extreme damage. Ultra rare. Boss fights only."},
        {"Flamethrower Fuel", "Fire weapon", "Area damage. Rare. Good vs groups."}
    };
    for (int i = 0; i < 9; i++) {
        json.addObjectInArray();
        json.addString("ammo_type", ammo[i][0]);
        json.addString("rarity", ammo[i][1]);
        json.addString("notes", ammo[i][2]);
        json.endObjectInArray();
    }
    json.endArray();
    
    // Combat system
    json.startArray("combat_mechanics");
    json.addObjectInArray();
    json.addString("mechanic", "Action Points (AP)");
    json.addString("description", "Each action costs AP. Movement, attacks, item use all consume AP. Turn ends when out of AP or you wait.");
    json.endObjectInArray();
    json.addObjectInArray();
    json.addString("mechanic", "Hit Chance");
    json.addString("description", "Based on weapon skill, perception, distance, and target's armor class. Higher skills = better accuracy.");
    json.endObjectInArray();
    json.addObjectInArray();
    json.addString("mechanic", "Critical Hits");
    json.addString("description", "Chance for extra damage. Affected by Luck stat and Better Criticals perk.");
    json.endObjectInArray();
    json.addObjectInArray();
    json.addString("mechanic", "Armor");
    json.addString("description", "Reduces damage taken. Leather < Metal < Combat < Power Armor. Check AC stat.");
    json.endObjectInArray();
    json.endArray();
    
    // SPECIAL system
    json.startArray("special_stats");
    const char* special[][2] = {
        {"Strength", "Affects melee damage and carry weight. Important for combat characters."},
        {"Perception", "Affects ranged accuracy and awareness. Critical for ranged combat."},
        {"Endurance", "Affects HP and resistances. More HP = more survivability."},
        {"Charisma", "Affects NPC reactions and companions. Higher = better dialogue options."},
        {"Intelligence", "Affects skill points per level. Higher = faster character progression."},
        {"Agility", "Affects AP and sequence. More AP = more actions per turn in combat."},
        {"Luck", "Affects critical chance and random encounters. General purpose stat."}
    };
    for (int i = 0; i < 7; i++) {
        json.addObjectInArray();
        json.addString("stat", special[i][0]);
        json.addString("effect", special[i][1]);
        json.endObjectInArray();
    }
    json.endArray();
    
    // Skill usage guide
    json.startArray("skill_guide");
    const char* skills[][2] = {
        {"Small Guns", "Most common weapons. Prioritize early game. Pistols, rifles, SMGs."},
        {"Energy Weapons", "Late game weapons. Laser/plasma rifles. Very powerful but rare ammo."},
        {"Melee/Unarmed", "Close combat. Useful when low on ammo. Knives, sledgehammers, fists."},
        {"First Aid", "Heal without items. Use between combats to save Stimpaks."},
        {"Doctor", "Heal critical injuries. More effective than First Aid but slower."},
        {"Sneak", "Avoid combat and get better positioning. Useful for stealing and reconnaissance."},
        {"Lockpick", "Open locked containers and doors. Essential for accessing loot and shortcuts."},
        {"Speech", "Better dialogue options. Can avoid combat, get better prices, complete quests peacefully."},
        {"Barter", "Better trading prices. Save caps by improving this skill."},
        {"Science", "Use computers and technology. Required for some quests and shortcuts."},
        {"Repair", "Fix broken items and machinery. Useful for equipment maintenance."}
    };
    for (int i = 0; i < 11; i++) {
        json.addObjectInArray();
        json.addString("skill", skills[i][0]);
        json.addString("strategy", skills[i][1]);
        json.endObjectInArray();
    }
    json.endArray();
    
    // Survival tips
    json.startArray("survival_tips");
    const char* tips[] = {
        "Save frequently! Use multiple save slots.",
        "Stimpaks are life-savers. Always carry several.",
        "Higher skills mean better success rates. Invest in key skills early.",
        "Check your HP after every combat. Heal before exploring further.",
        "Ammunition is precious. Aim for high hit-chance shots.",
        "Talk to NPCs. They provide quests, information, and trading opportunities.",
        "Explore thoroughly. Containers often have useful items and ammo.",
        "Sneaking can avoid dangerous encounters. Use it when outnumbered.",
        "In combat, positioning matters. Use cover and distance.",
        "Action Points determine how much you can do per turn. Manage them wisely.",
        "Experience is gained from quests, combat, and exploration. Level up gives skill points.",
        "Different weapons are effective against different enemies. Experiment.",
        "Read item descriptions. They explain what items do and how to use them.",
        "Sequence stat determines turn order. Higher sequence = act first in combat.",
        "Armor Class (AC) makes you harder to hit. Higher AC = fewer hits taken."
    };
    for (int i = 0; i < 15; i++) {
        json.addObjectInArray();
        json.addInt("tip_id", i + 1);
        json.addString("tip", tips[i]);
        json.endObjectInArray();
    }
    json.endArray();
    
    // Object types explanation
    json.startArray("object_types");
    const char* objTypes[][2] = {
        {"0", "Item - Can be picked up and used. Weapons, armor, consumables, quest items."},
        {"1", "Critter - Living beings. Can be friendly NPCs or hostile enemies. Can be talked to or attacked."},
        {"2", "Scenery - Environment objects. Doors, containers, furniture. Some can be interacted with."},
        {"3", "Wall - Impassable terrain. Blocks movement and line of sight."},
        {"5", "Misc - Miscellaneous objects. Varies by context."}
    };
    for (int i = 0; i < 5; i++) {
        json.addObjectInArray();
        json.addString("type_id", objTypes[i][0]);
        json.addString("description", objTypes[i][1]);
        json.endObjectInArray();
    }
    json.endArray();
    
    // Decision making guide
    json.startArray("decision_making");
    json.addObjectInArray();
    json.addString("situation", "Low HP");
    json.addString("action", "Use Stimpak (PID 40) immediately or retreat from combat.");
    json.endObjectInArray();
    json.addObjectInArray();
    json.addString("situation", "In Combat");
    json.addString("action", "Attack if you have good hit chance, or move to better position, or use item to heal.");
    json.endObjectInArray();
    json.addObjectInArray();
    json.addString("situation", "Exploring");
    json.addString("action", "Move to nearby objects to investigate. Pick up items. Talk to NPCs (type 1 objects).");
    json.endObjectInArray();
    json.addObjectInArray();
    json.addString("situation", "Low Ammo");
    json.addString("action", "Search containers, avoid unnecessary combat, consider melee weapons.");
    json.endObjectInArray();
    json.addObjectInArray();
    json.addString("situation", "Enemy Nearby");
    json.addString("action", "If strong: attack. If weak: run or sneak. If talking is possible: try Speech.");
    json.endObjectInArray();
    json.endArray();
    
    // Game world context
    json.addString("game_world", "Set in post-nuclear California wasteland. Vault dweller from Vault 13 seeking water chip and fighting Super Mutant threat.");
    json.addString("primary_goal", "Initially find water chip for vault. Later, stop the Master's army of Super Mutants.");
    json.addString("gameplay_style", "Turn-based tactical RPG with exploration, combat, quests, and dialogue choices. Character build and skills matter greatly.");
    
    // How to control the game via API
    json.startArray("api_control_guide");
    json.addObjectInArray();
    json.addString("step", "1. Read Game State");
    json.addString("description", "Read ai_state.json file every frame to get current game state including position, HP, nearby objects, inventory.");
    json.addString("file", "ai_state.json");
    json.endObjectInArray();
    
    json.addObjectInArray();
    json.addString("step", "2. Analyze State");
    json.addString("description", "Check player HP, nearby objects, combat state, available items. Determine what action to take.");
    json.addString("example", "If HP < 30% and have Stimpak (PID 40), use it. If enemy nearby and in combat, attack.");
    json.endObjectInArray();
    
    json.addObjectInArray();
    json.addString("step", "3. Write Action");
    json.addString("description", "Write a JSON file with your chosen action. File is deleted after being read by game.");
    json.addString("file", "ai_action.json");
    json.endObjectInArray();
    
    json.addObjectInArray();
    json.addString("step", "4. Check Result");
    json.addString("description", "In next state update, check last_action_result field to see if action succeeded or failed.");
    json.addString("example", "success: moved to tile 20150' or 'error: not enough AP'");
    json.endObjectInArray();
    json.endArray();
    
    // Detailed action examples with JSON
    json.startArray("action_examples");
    
    json.addObjectInArray();
    json.addString("action_name", "Move to Location");
    json.addString("when_to_use", "Exploring, repositioning, approaching objects or NPCs");
    json.addString("json_format", "{\"action\": \"move\", \"target_tile\": 20150}");
    json.addString("how_to_choose_tile", "Look at nearby_objects array in state. Get tile number of interesting object. Or add/subtract from player_tile to move nearby.");
    json.addString("tips", "In combat costs AP. Out of combat is free. Can't move through walls or other obstacles.");
    json.endObjectInArray();
    
    json.addObjectInArray();
    json.addString("action_name", "Attack Enemy");
    json.addString("when_to_use", "In combat, when enemy is visible and you have weapon");
    json.addString("json_format", "{\"action\": \"attack\", \"target_tile\": 20105}");
    json.addString("how_to_choose_tile", "Find critter in nearby_objects with is_dead: false. Use their tile number as target.");
    json.addString("tips", "Costs AP. Check you have enough AP. Better to attack when close for higher hit chance.");
    json.endObjectInArray();
    
    json.addObjectInArray();
    json.addString("action_name", "Use Healing Item");
    json.addString("when_to_use", "When HP is low (below 50% of max)");
    json.addString("json_format", "{\"action\": \"use_item\", \"target_pid\": 40}");
    json.addString("how_to_find_pid", "Look in inventory array for item with name 'Stimpak'. Use its pid value (usually 40).");
    json.addString("tips", "Always heal before HP gets too low. Stimpak (40) heals ~15-20 HP. Super Stimpak (144) heals more.");
    json.endObjectInArray();
    
    json.addObjectInArray();
    json.addString("action_name", "Pick Up Item");
    json.addString("when_to_use", "When valuable item on ground nearby (ammo, weapons, caps, stimpaks)");
    json.addString("json_format", "{\"action\": \"pickup\", \"target_tile\": 20105, \"target_pid\": 41}");
    json.addString("how_to_find", "Look in nearby_objects for type: 0 (items). Use tile and pid from that object.");
    json.addString("tips", "Always pick up Stimpaks, ammo, caps. Check carry_weight stat to avoid overencumbrance.");
    json.endObjectInArray();
    
    json.addObjectInArray();
    json.addString("action_name", "Wait/Skip Turn");
    json.addString("when_to_use", "In combat when can't do anything useful, or to pass time");
    json.addString("json_format", "{\"action\": \"wait\"}");
    json.addString("tips", "Ends your turn in combat. Use when out of AP or no good action available.");
    json.endObjectInArray();
    json.endArray();
    
    // State interpretation guide
    json.startArray("state_interpretation");
    
    json.addObjectInArray();
    json.addString("field", "hit_points / max_hit_points");
    json.addString("meaning", "Current and maximum health. If hit_points < 30% of max, heal immediately!");
    json.addString("action", "If low, use Stimpak (PID 40) or retreat from combat.");
    json.endObjectInArray();
    
    json.addObjectInArray();
    json.addString("field", "action_points / max_action_points");
    json.addString("meaning", "AP available this turn. Each action costs AP. More agility = more max AP.");
    json.addString("action", "Track AP before acting. If low, choose cheap actions or wait.");
    json.endObjectInArray();
    
    json.addObjectInArray();
    json.addString("field", "in_combat");
    json.addString("meaning", "true = in turn-based combat, false = exploring freely");
    json.addString("action", "In combat: be tactical, manage AP. Out of combat: explore freely, heal up.");
    json.endObjectInArray();
    
    json.addObjectInArray();
    json.addString("field", "nearby_objects[]");
    json.addString("meaning", "Objects within 10 tiles. Includes NPCs (type 1), items (type 0), scenery (type 2).");
    json.addString("action", "Check distance to prioritize. Interact with close objects first. Attack type 1 if hostile.");
    json.endObjectInArray();
    
    json.addObjectInArray();
    json.addString("field", "inventory[]");
    json.addString("meaning", "Items you're carrying. Check for Stimpaks (40), ammo, weapons, tools.");
    json.addString("action", "Use consumables when needed. Track quantities. Pick up more when low.");
    json.endObjectInArray();
    
    json.addObjectInArray();
    json.addString("field", "skills[]");
    json.addString("meaning", "Your skill levels. Higher = better success rate. Small Guns, Speech, Lockpick most useful early.");
    json.addString("action", "Focus on improving useful skills when leveling. Check which skills support your strategy.");
    json.endObjectInArray();
    
    json.addObjectInArray();
    json.addString("field", "last_action_result");
    json.addString("meaning", "Feedback from your last action. 'success: ...' or 'error: ...'");
    json.addString("action", "Read this to know if your action worked. If error, try different approach.");
    json.endObjectInArray();
    
    json.addObjectInArray();
    json.addString("field", "recent_events[]");
    json.addString("meaning", "Last 50 events. Shows what happened recently (damage, level ups, actions).");
    json.addString("action", "Review to understand recent game state changes. Learn from mistakes.");
    json.endObjectInArray();
    json.endArray();
    
    // Decision tree examples
    json.startArray("decision_trees");
    
    json.addObjectInArray();
    json.addString("scenario", "Combat Situation");
    json.addString("decision_logic", "IF in_combat AND hit_points < 30% max: use Stimpak. ELIF enemy nearby: attack closest. ELIF action_points low: wait. ELSE: move closer to enemy.");
    json.endObjectInArray();
    
    json.addObjectInArray();
    json.addString("scenario", "Exploration");
    json.addString("decision_logic", "IF item nearby (type 0): pickup. ELIF NPC nearby (type 1): move towards to interact. ELIF unexplored direction: move that direction. ELSE: move randomly.");
    json.endObjectInArray();
    
    json.addObjectInArray();
    json.addString("scenario", "Low Health");
    json.addString("decision_logic", "IF Stimpak in inventory: use it. ELIF not in combat: search for items. ELIF in combat: retreat/wait. ELSE: hope for the best.");
    json.endObjectInArray();
    
    json.addObjectInArray();
    json.addString("scenario", "First Turn");
    json.addString("decision_logic", "Read state fully. Check HP, inventory, nearby objects. Orient yourself. Plan next few actions based on state.");
    json.endObjectInArray();
    json.endArray();
    
    // Common mistakes to avoid
    json.startArray("common_mistakes");
    const char* mistakes[] = {
        "Don't spam actions. Wait for last_action_result before sending next action.",
        "Don't ignore HP. Heal when HP < 50%. Don't wait until critical.",
        "Don't waste AP on unnecessary movement in combat. Each tile costs AP.",
        "Don't attack without checking if you have ammo or weapon equipped.",
        "Don't pick up everything. Check carry_weight. Heavy items slow you down.",
        "Don't move randomly. Check nearby_objects first to find interesting targets.",
        "Don't forget to check in_combat status. Combat has different rules.",
        "Don't send invalid tile numbers. Must be valid reachable location.",
        "Don't use items you don't have. Check inventory[] first.",
        "Don't attack friendly NPCs. Check if critter is hostile before attacking.",
        "Don't ignore action feedback. Read last_action_result to learn.",
        "Don't rush. Take time to analyze state before acting.",
        "Don't forget AP management in combat. Track action_points carefully.",
        "Don't move into unexplored areas at low HP. Heal first, then explore.",
        "Don't waste consumables. Use healing items only when needed."
    };
    for (int i = 0; i < 15; i++) {
        json.addObjectInArray();
        json.addString("mistake", mistakes[i]);
        json.endObjectInArray();
    }
    json.endArray();
    
    // Quick reference
    json.addString("quick_reference", "Read ai_state.json -> Analyze HP, AP, nearby objects -> Decide action -> Write ai_action.json -> Check result in next state");
    json.addString("most_important", "Keep HP above 50%. Use Stimpak (PID 40) when low. In combat: manage AP carefully. Explore: pickup items, talk to NPCs.");
    json.addString("cooldown_info", "100ms between actions. Don't send actions faster than this or you'll get 'error: cooldown active'.");
    
    // JSON Format Specifications
    json.startArray("json_schemas");
    
    // Input: What you READ (ai_state.json)
    json.addObjectInArray();
    json.addString("file", "ai_state.json");
    json.addString("direction", "INPUT - Read this file to understand game state");
    json.addString("format", "Valid JSON object with specific fields");
    json.addString("schema", "{"
        "\"player_tile\": <integer>, "
        "\"player_elevation\": <integer 0-2>, "
        "\"player_rotation\": <integer 0-5>, "
        "\"hit_points\": <integer>, "
        "\"max_hit_points\": <integer>, "
        "\"action_points\": <integer>, "
        "\"max_action_points\": <integer>, "
        "\"level\": <integer>, "
        "\"experience\": <integer>, "
        "\"in_combat\": <boolean>, "
        "\"strength\": <integer 1-10>, "
        "\"perception\": <integer 1-10>, "
        "\"endurance\": <integer 1-10>, "
        "\"charisma\": <integer 1-10>, "
        "\"intelligence\": <integer 1-10>, "
        "\"agility\": <integer 1-10>, "
        "\"luck\": <integer 1-10>, "
        "\"skills\": [{\"name\": <string>, \"value\": <integer>}, ...], "
        "\"perks\": [{\"name\": <string>, \"level\": <integer>}, ...], "
        "\"map_name\": <string>, "
        "\"nearby_objects\": [{\"tile\": <int>, \"distance\": <int>, \"type\": <int>, \"pid\": <int>, \"name\": <string>, \"is_dead\": <bool>, \"hp\": <int>}, ...], "
        "\"inventory\": [{\"pid\": <int>, \"quantity\": <int>, \"name\": <string>}, ...], "
        "\"total_damage_dealt\": <integer>, "
        "\"total_kills\": <integer>, "
        "\"session_time_seconds\": <integer>, "
        "\"last_action_result\": <string>, "
        "\"recent_events\": [{\"event\": <string>}, ...]}");
    json.addString("reading_tips", "Parse as JSON. Access fields by name. All field names are strings. Values are typed (int/bool/string/array).");
    json.endObjectInArray();
    
    // Output: What you WRITE (ai_action.json)
    json.addObjectInArray();
    json.addString("file", "ai_action.json");
    json.addString("direction", "OUTPUT - Write this file to send action to game");
    json.addString("format", "Must be valid JSON object with 'action' field");
    json.addString("required_field", "action - String specifying action type");
    json.addString("optional_fields", "target_tile (int), target_pid (int) - depends on action");
    json.endObjectInArray();
    
    json.endArray();
    
    // Detailed action JSON formats
    json.startArray("action_json_formats");
    
    json.addObjectInArray();
    json.addString("action_type", "move");
    json.addString("required_fields", "action, target_tile");
    json.addString("json_example", "{\"action\": \"move\", \"target_tile\": 20150}");
    json.addString("field_types", "action: string, target_tile: integer");
    json.addString("validation", "target_tile must be valid integer 0-39999. No quotes around numbers!");
    json.endObjectInArray();
    
    json.addObjectInArray();
    json.addString("action_type", "attack");
    json.addString("required_fields", "action, target_tile");
    json.addString("json_example", "{\"action\": \"attack\", \"target_tile\": 20105}");
    json.addString("field_types", "action: string, target_tile: integer");
    json.addString("validation", "target_tile must contain a critter. Get from nearby_objects where type=1.");
    json.endObjectInArray();
    
    json.addObjectInArray();
    json.addString("action_type", "use_item");
    json.addString("required_fields", "action, target_pid");
    json.addString("json_example", "{\"action\": \"use_item\", \"target_pid\": 40}");
    json.addString("field_types", "action: string, target_pid: integer");
    json.addString("validation", "target_pid must exist in inventory array. Get pid value from inventory item.");
    json.endObjectInArray();
    
    json.addObjectInArray();
    json.addString("action_type", "pickup");
    json.addString("required_fields", "action, target_tile, target_pid");
    json.addString("json_example", "{\"action\": \"pickup\", \"target_tile\": 20105, \"target_pid\": 41}");
    json.addString("field_types", "action: string, target_tile: integer, target_pid: integer");
    json.addString("validation", "Both tile and pid must match an item in nearby_objects where type=0.");
    json.endObjectInArray();
    
    json.addObjectInArray();
    json.addString("action_type", "wait");
    json.addString("required_fields", "action");
    json.addString("json_example", "{\"action\": \"wait\"}");
    json.addString("field_types", "action: string");
    json.addString("validation", "No additional fields needed. Simple action.");
    json.endObjectInArray();
    
    json.endArray();
    
    // JSON syntax rules
    json.startArray("json_syntax_rules");
    
    json.addObjectInArray();
    json.addString("rule", "Use double quotes for strings");
    json.addString("correct", "\"action\": \"move\"");
    json.addString("wrong", "'action': 'move' or action: move");
    json.endObjectInArray();
    
    json.addObjectInArray();
    json.addString("rule", "Numbers have NO quotes");
    json.addString("correct", "\"target_tile\": 20150");
    json.addString("wrong", "\"target_tile\": \"20150\"");
    json.endObjectInArray();
    
    json.addObjectInArray();
    json.addString("rule", "Booleans are lowercase true/false");
    json.addString("correct", "\"in_combat\": true");
    json.addString("wrong", "\"in_combat\": True or \"in_combat\": \"true\"");
    json.endObjectInArray();
    
    json.addObjectInArray();
    json.addString("rule", "Field names must have quotes");
    json.addString("correct", "{\"action\": \"move\"}");
    json.addString("wrong", "{action: \"move\"}");
    json.endObjectInArray();
    
    json.addObjectInArray();
    json.addString("rule", "Use commas between fields");
    json.addString("correct", "{\"action\": \"move\", \"target_tile\": 100}");
    json.addString("wrong", "{\"action\": \"move\" \"target_tile\": 100}");
    json.endObjectInArray();
    
    json.addObjectInArray();
    json.addString("rule", "NO trailing comma after last field");
    json.addString("correct", "{\"action\": \"wait\"}");
    json.addString("wrong", "{\"action\": \"wait\",}");
    json.endObjectInArray();
    
    json.addObjectInArray();
    json.addString("rule", "Curly braces for objects");
    json.addString("correct", "{\"action\": \"move\", \"target_tile\": 100}");
    json.addString("note", "Start with { and end with }");
    json.endObjectInArray();
    
    json.addObjectInArray();
    json.addString("rule", "Square brackets for arrays");
    json.addString("example", "\"inventory\": [{\"pid\": 40}, {\"pid\": 41}]");
    json.addString("note", "Arrays contain multiple items. Access by index.");
    json.endObjectInArray();
    
    json.endArray();
    
    // Parsing instructions
    json.startArray("parsing_instructions");
    
    json.addObjectInArray();
    json.addString("step", "1. Load ai_state.json");
    json.addString("instruction", "Read entire file as text. Parse as JSON object. Most languages have JSON.parse() or equivalent.");
    json.addString("example_python", "import json; state = json.load(open('ai_state.json'))");
    json.endObjectInArray();
    
    json.addObjectInArray();
    json.addString("step", "2. Access fields");
    json.addString("instruction", "Use dot notation or bracket notation to access fields.");
    json.addString("example_python", "hp = state['hit_points']; in_combat = state['in_combat']");
    json.endObjectInArray();
    
    json.addObjectInArray();
    json.addString("step", "3. Iterate arrays");
    json.addString("instruction", "Loop through arrays to find specific items or objects.");
    json.addString("example_python", "for obj in state['nearby_objects']: if obj['type'] == 1: print('Found NPC')");
    json.endObjectInArray();
    
    json.addObjectInArray();
    json.addString("step", "4. Check field existence");
    json.addString("instruction", "Some fields may not exist. Check before accessing.");
    json.addString("example_python", "if 'hp' in obj: enemy_hp = obj['hp']");
    json.endObjectInArray();
    
    json.addObjectInArray();
    json.addString("step", "5. Create action object");
    json.addString("instruction", "Build a Python dict or JavaScript object with action fields.");
    json.addString("example_python", "action = {'action': 'move', 'target_tile': 20150}");
    json.endObjectInArray();
    
    json.addObjectInArray();
    json.addString("step", "6. Write action to file");
    json.addString("instruction", "Convert object to JSON string and write to ai_action.json.");
    json.addString("example_python", "import json; json.dump(action, open('ai_action.json', 'w'))");
    json.endObjectInArray();
    
    json.endArray();
    
    // Common parsing errors
    json.startArray("common_parsing_errors");
    
    json.addObjectInArray();
    json.addString("error", "Numbers as strings");
    json.addString("problem", "Writing \"target_tile\": \"20150\" instead of \"target_tile\": 20150");
    json.addString("fix", "Remove quotes around numbers. Only strings need quotes.");
    json.endObjectInArray();
    
    json.addObjectInArray();
    json.addString("error", "Single quotes instead of double");
    json.addString("problem", "{'action': 'move'} - JSON requires double quotes");
    json.addString("fix", "Use double quotes: {\"action\": \"move\"}");
    json.endObjectInArray();
    
    json.addObjectInArray();
    json.addString("error", "Missing comma");
    json.addString("problem", "{\"action\": \"move\" \"target_tile\": 100}");
    json.addString("fix", "Add comma: {\"action\": \"move\", \"target_tile\": 100}");
    json.endObjectInArray();
    
    json.addObjectInArray();
    json.addString("error", "Trailing comma");
    json.addString("problem", "{\"action\": \"wait\",}");
    json.addString("fix", "Remove last comma: {\"action\": \"wait\"}");
    json.endObjectInArray();
    
    json.addObjectInArray();
    json.addString("error", "Wrong field names");
    json.addString("problem", "Writing 'type' instead of 'action' as main field");
    json.addString("fix", "Use exact field names: 'action', 'target_tile', 'target_pid'");
    json.endObjectInArray();
    
    json.addObjectInArray();
    json.addString("error", "Missing required fields");
    json.addString("problem", "{\"action\": \"move\"} without target_tile");
    json.addString("fix", "Include all required fields for that action type");
    json.endObjectInArray();
    
    json.endArray();
    
    // Complete example workflow
    json.addString("complete_example_workflow", 
        "1. Read ai_state.json and parse JSON. "
        "2. Check hit_points: if < 30, find Stimpak in inventory (name='Stimpak'), get its pid. "
        "3. Create action: {\"action\": \"use_item\", \"target_pid\": 40}. "
        "4. Write to ai_action.json using JSON.dump/stringify. "
        "5. Wait 100ms (cooldown). "
        "6. Read next state update. "
        "7. Check last_action_result: should say 'success: used Stimpak'. "
        "8. Verify hit_points increased. "
        "9. Continue with next decision.");
    
    json.endObject();
    
    // Write to file
    char tempPath[256];
    snprintf(tempPath, sizeof(tempPath), "%s.tmp", kKnowledgeFilePath);
    
    FILE* fp = fopen(tempPath, "w");
    if (fp) {
        fwrite(json.c_str(), 1, json.length(), fp);
        fclose(fp);
        rename(tempPath, kKnowledgeFilePath);
        gKnowledgeWritten = true;
    }
}

// Write current game state to JSON file
static void writeGameState() {
    if (!obj_dude) {
        return;
    }
    
    // Track events
    int currentHP = critter_get_hits(obj_dude);
    int currentLevel = stat_pc_get(PC_STAT_LEVEL);
    
    if (gLastHitPoints > 0 && currentHP < gLastHitPoints) {
        int damage = gLastHitPoints - currentHP;
        char eventDesc[128];
        snprintf(eventDesc, sizeof(eventDesc), "Took %d damage (HP: %d->%d)", damage, gLastHitPoints, currentHP);
        addEvent("damage_taken", eventDesc);
    }
    
    if (currentLevel > gLastLevel && gLastLevel > 0) {
        char eventDesc[128];
        snprintf(eventDesc, sizeof(eventDesc), "Level up! Now level %d", currentLevel);
        addEvent("level_up", eventDesc);
        addMilestone(eventDesc);
    }
    
    gLastHitPoints = currentHP;
    gLastLevel = currentLevel;
    
    JsonWriter json;
    json.startObject();
    
    // Player position
    json.addInt("player_tile", obj_dude->tile);
    json.addInt("player_elevation", obj_dude->elevation);
    json.addInt("player_rotation", obj_dude->rotation);
    
    // Player stats
    json.addInt("hit_points", currentHP);
    json.addInt("max_hit_points", stat_level(obj_dude, STAT_MAXIMUM_HIT_POINTS));
    json.addInt("action_points", obj_dude->data.critter.combat.ap);
    json.addInt("max_action_points", stat_level(obj_dude, STAT_MAXIMUM_ACTION_POINTS));
    json.addInt("level", currentLevel);
    json.addInt("experience", stat_pc_get(PC_STAT_EXPERIENCE));
    json.addInt("armor_class", stat_level(obj_dude, STAT_ARMOR_CLASS));
    json.addInt("sequence", stat_level(obj_dude, STAT_SEQUENCE));
    json.addInt("carry_weight", stat_level(obj_dude, STAT_CARRY_WEIGHT));
    json.addInt("melee_damage", stat_level(obj_dude, STAT_MELEE_DAMAGE));
    
    // Additional derived stats
    json.addInt("healing_rate", stat_level(obj_dude, STAT_HEALING_RATE));
    json.addInt("critical_chance", stat_level(obj_dude, STAT_CRITICAL_CHANCE));
    json.addInt("damage_resistance", stat_level(obj_dude, STAT_DAMAGE_RESISTANCE));
    json.addInt("radiation_resistance", stat_level(obj_dude, STAT_RADIATION_RESISTANCE));
    json.addInt("poison_resistance", stat_level(obj_dude, STAT_POISON_RESISTANCE));
    
    // Karma and reputation (PC-specific stats)
    json.addInt("karma", stat_pc_get(PC_STAT_KARMA));
    json.addInt("reputation", stat_pc_get(PC_STAT_REPUTATION));
    
    // Character identity
    json.addInt("age", stat_level(obj_dude, STAT_AGE));
    json.addInt("gender", stat_level(obj_dude, STAT_GENDER)); // 0=male, 1=female
    
    // Character name
    char* playerName = object_name(obj_dude);
    if (playerName && strlen(playerName) > 0) {
        json.addString("character_name", playerName);
    }
    
    // Combat state
    json.addBool("in_combat", isInCombat());
    
    // Primary stats (SPECIAL)
    json.addInt("strength", stat_level(obj_dude, STAT_STRENGTH));
    json.addInt("perception", stat_level(obj_dude, STAT_PERCEPTION));
    json.addInt("endurance", stat_level(obj_dude, STAT_ENDURANCE));
    json.addInt("charisma", stat_level(obj_dude, STAT_CHARISMA));
    json.addInt("intelligence", stat_level(obj_dude, STAT_INTELLIGENCE));
    json.addInt("agility", stat_level(obj_dude, STAT_AGILITY));
    json.addInt("luck", stat_level(obj_dude, STAT_LUCK));
    
    // Skills
    json.startArray("skills");
    const char* skillNames[] = {
        "Small Guns", "Big Guns", "Energy Weapons", "Unarmed", "Melee Weapons",
        "Throwing", "First Aid", "Doctor", "Sneak", "Lockpick", "Steal",
        "Traps", "Science", "Repair", "Speech", "Barter", "Gambling", "Outdoorsman"
    };
    for (int i = 0; i < SKILL_COUNT; i++) {
        json.addObjectInArray();
        json.addString("name", skillNames[i]);
        json.addInt("value", skill_level(obj_dude, i));
        json.endObjectInArray();
    }
    json.endArray();
    
    // Perks (up to 20)
    json.startArray("perks");
    int perks[128]; // Max perks
    int perkCount = perk_make_list(perks);
    for (int i = 0; i < perkCount && i < 20; i++) {
        json.addObjectInArray();
        json.addString("name", perk_name(perks[i]));
        json.addInt("level", perk_level(perks[i]));
        json.endObjectInArray();
    }
    json.endArray();
    
    // Traits
    json.startArray("traits");
    int trait1, trait2;
    trait_get(&trait1, &trait2);
    
    if (trait1 != -1) {
        json.addObjectInArray();
        json.addString("name", trait_name(trait1));
        json.addString("description", trait_description(trait1));
        json.endObjectInArray();
    }
    
    if (trait2 != -1) {
        json.addObjectInArray();
        json.addString("name", trait_name(trait2));
        json.addString("description", trait_description(trait2));
        json.endObjectInArray();
    }
    json.endArray();
    
    // Map info
    int mapIndex = map_get_index_number();
    if (mapIndex != -1) {
        char* mapName = map_get_short_name(mapIndex);
        if (mapName) {
            json.addString("map_name", mapName);
        }
    }
    
    // Nearby objects (within reasonable range)
    json.startArray("nearby_objects");
    
    Object* obj = obj_find_first_at(obj_dude->elevation);
    int count = 0;
    while (obj != NULL && count < 100) { // Limit to 100 objects
        if (obj != obj_dude) {
            int distance = obj_dist(obj_dude, obj);
            
            if (distance <= 10) { // Only nearby objects
                json.addObjectInArray();
                json.addInt("tile", obj->tile);
                json.addInt("distance", distance);
                json.addInt("type", FID_TYPE(obj->fid));
                json.addInt("pid", obj->pid);
                
                char* objName = object_name(obj);
                if (objName) {
                    json.addString("name", objName);
                }
                
                // Check if it's a critter
                if (FID_TYPE(obj->fid) == OBJ_TYPE_CRITTER) {
                    json.addBool("is_dead", critter_is_dead(obj));
                    if (!critter_is_dead(obj)) {
                        json.addInt("hp", critter_get_hits(obj));
                    }
                }
                
                json.endObjectInArray();
                count++;
            }
        }
        obj = obj_find_next_at();
    }
    json.endArray();
    
    // Inventory (first 20 items)
    json.startArray("inventory");
    Inventory* inventory = &(obj_dude->data.critter.inventory);
    int invCount = 0;
    for (int i = 0; i < inventory->length && invCount < 20; i++) {
        InventoryItem* item = &(inventory->items[i]);
        if (item->item) {
            json.addObjectInArray();
            json.addInt("pid", item->item->pid);
            json.addInt("quantity", item->quantity);
            
            char* itemName = object_name(item->item);
            if (itemName) {
                json.addString("name", itemName);
            }
            
            json.endObjectInArray();
            invCount++;
        }
    }
    json.endArray();
    
    // Equipped items
    json.startObject("equipped");
    
    Object* rightHand = inven_right_hand(obj_dude);
    if (rightHand) {
        json.startObject("right_hand");
        json.addInt("pid", rightHand->pid);
        char* rightName = object_name(rightHand);
        if (rightName) {
            json.addString("name", rightName);
        }
        json.endObject();
    }
    
    Object* leftHand = inven_left_hand(obj_dude);
    if (leftHand) {
        json.startObject("left_hand");
        json.addInt("pid", leftHand->pid);
        char* leftName = object_name(leftHand);
        if (leftName) {
            json.addString("name", leftName);
        }
        json.endObject();
    }
    
    Object* armor = inven_worn(obj_dude);
    if (armor) {
        json.startObject("armor");
        json.addInt("pid", armor->pid);
        char* armorName = object_name(armor);
        if (armorName) {
            json.addString("name", armorName);
        }
        json.endObject();
    }
    
    json.endObject(); // equipped
    
    // Quest tracking (key quest global variables)
    json.startObject("quests");
    
    // Main story quests
    json.addInt("find_water_chip", game_get_global_var(GVAR_FIND_WATER_CHIP));
    json.addInt("destroy_vats", game_get_global_var(GVAR_DESTROY_VATS));
    json.addInt("destroy_master", game_get_global_var(GVAR_DESTROY_MASTER));
    json.addInt("days_to_vault13_discovery", game_get_global_var(GVAR_DAYS_TO_VAULT13_DISCOVERY));
    json.addInt("vault_water_days", game_get_global_var(GVAR_VAULT_WATER));
    
    // Major side quests
    json.addInt("rescue_tandi", game_get_global_var(GVAR_RESCUE_TANDI));
    json.addInt("tandi_status", game_get_global_var(GVAR_TANDI_STATUS));
    json.addInt("kill_radscorpions", game_get_global_var(GVAR_KILL_RADSCORPIONS));
    json.addInt("kill_deathclaw", game_get_global_var(GVAR_KILL_DEATHCLAW));
    json.addInt("capture_gizmo", game_get_global_var(GVAR_CAPTURE_GIZMO));
    json.addInt("kill_killian", game_get_global_var(GVAR_KILL_KILLIAN));
    json.addInt("missing_caravan", game_get_global_var(GVAR_MISSING_CARAVAN));
    json.addInt("steal_necklace", game_get_global_var(GVAR_STEAL_NECKLACE));
    json.addInt("become_an_initiate", game_get_global_var(GVAR_BECOME_AN_INITIATE));
    json.addInt("find_lost_initiate", game_get_global_var(GVAR_FIND_LOST_INITIATE));
    json.addInt("kill_merchant", game_get_global_var(GVAR_KILL_MERCHANT));
    json.addInt("kill_jain", game_get_global_var(GVAR_KILL_JAIN));
    json.addInt("kill_super_mutants", game_get_global_var(GVAR_KILL_SUPER_MUTANTS));
    json.addInt("fix_necropolis_pump", game_get_global_var(GVAR_NECROP_WATER_PUMP_FIXED));
    json.addInt("necropolis_water_chip_taken", game_get_global_var(GVAR_NECROP_WATER_CHIP_TAKEN));
    json.addInt("gang_war", game_get_global_var(GVAR_GANG_WAR));
    json.addInt("destroy_followers", game_get_global_var(GVAR_DESTROY_FOLLOWERS));
    json.addInt("fix_farm", game_get_global_var(GVAR_FIX_FARM));
    json.addInt("save_sinthia", game_get_global_var(GVAR_SAVE_SINTHIA));
    json.addInt("cure_jarvis", game_get_global_var(GVAR_CURE_JARVIS));
    json.addInt("make_antidote", game_get_global_var(GVAR_MAKE_ANTIDOTE));
    
    json.endObject();
    
    // Town reputation (enemy status with each faction)
    json.startObject("town_reputation");
    json.addInt("vault_13", game_get_global_var(GVAR_ENEMY_VAULT_13));
    json.addInt("shady_sands", game_get_global_var(GVAR_ENEMY_SHADY_SANDS));
    json.addInt("junktown", game_get_global_var(GVAR_ENEMY_JUNKTOWN));
    json.addInt("hub", game_get_global_var(GVAR_ENEMY_HUB));
    json.addInt("necropolis", game_get_global_var(GVAR_ENEMY_NECROPOLIS));
    json.addInt("brotherhood", game_get_global_var(GVAR_ENEMY_BROTHERHOOD));
    json.addInt("adytum", game_get_global_var(GVAR_ENEMY_ADYTUM));
    json.addInt("rippers", game_get_global_var(GVAR_ENEMY_RIPPERS));
    json.addInt("blades", game_get_global_var(GVAR_ENEMY_BLADES));
    json.addInt("raiders", game_get_global_var(GVAR_ENEMY_RAIDERS));
    json.addInt("cathedral", game_get_global_var(GVAR_ENEMY_CATHEDRAL));
    json.addInt("followers", game_get_global_var(GVAR_ENEMY_FOLLOWERS));
    json.endObject();
    
    // Location discovery and visits
    json.startObject("locations_known");
    json.addInt("vault_13", game_get_global_var(GVAR_MARK_V13_1));
    json.addInt("vault_15", game_get_global_var(GVAR_MARK_V15_1));
    json.addInt("shady_sands", game_get_global_var(GVAR_MARK_SHADY_1));
    json.addInt("junktown", game_get_global_var(GVAR_MARK_JUNK_1));
    json.addInt("raiders", game_get_global_var(GVAR_MARK_RAIDERS_1));
    json.addInt("necropolis", game_get_global_var(GVAR_MARK_NECROP_1));
    json.addInt("hub", game_get_global_var(GVAR_MARK_HUB_1));
    json.addInt("brotherhood", game_get_global_var(GVAR_MARK_BROTHER_1));
    json.addInt("military_base", game_get_global_var(GVAR_MARK_BASE_1));
    json.addInt("glow", game_get_global_var(GVAR_MARK_GLOW_1));
    json.addInt("boneyard", game_get_global_var(GVAR_MARK_LA_1));
    json.addInt("cathedral", game_get_global_var(GVAR_MARK_CHILD_1));
    json.addInt("necropolis_visited", game_get_global_var(GVAR_NECROPOLIS_VISITED));
    json.addInt("necropolis_known", game_get_global_var(GVAR_NECROPOLIS_KNOWN));
    json.endObject();
    
    // Current player location (from worldmap)
    json.addInt("player_location_id", game_get_global_var(GVAR_PLAYER_LOCATION));
    
    // Streaming stats
    json.addInt("total_damage_dealt", gTotalDamageDealt);
    json.addInt("total_kills", gTotalKills);
    
    int elapsedTime = 0;
    if (gSessionStartTime > 0) {
        elapsedTime = (getCurrentTimeMs() - gSessionStartTime) / 1000; // seconds
    }
    json.addInt("session_time_seconds", elapsedTime);
    
    // Action feedback
    json.addString("last_action_result", gLastActionResult);
    
    // Context hints for AI
    json.startArray("ai_hints");
    
    // Health status hint
    float hpPercent = (float)currentHP / stat_level(obj_dude, STAT_MAXIMUM_HIT_POINTS) * 100.0f;
    if (hpPercent < 30.0f) {
        json.addObjectInArray();
        json.addString("priority", "CRITICAL");
        json.addString("hint", "HP very low! Use Stimpak immediately or retreat!");
        json.addString("suggested_action", "{\"action\": \"use_item\", \"target_pid\": 40}");
        json.endObjectInArray();
    } else if (hpPercent < 50.0f) {
        json.addObjectInArray();
        json.addString("priority", "HIGH");
        json.addString("hint", "HP below 50%. Consider healing soon.");
        json.addString("suggested_action", "{\"action\": \"use_item\", \"target_pid\": 40}");
        json.endObjectInArray();
    }
    
    // Combat hint
    if (isInCombat()) {
        json.addObjectInArray();
        json.addString("priority", "INFO");
        json.addString("hint", "In combat mode. Actions cost AP. Manage action points carefully.");
        json.addString("info", "Check action_points before acting. Each action has AP cost.");
        json.endObjectInArray();
        
        if (obj_dude->data.critter.combat.ap < 3) {
            json.addObjectInArray();
            json.addString("priority", "MEDIUM");
            json.addString("hint", "Low AP. Consider waiting to end turn.");
            json.addString("suggested_action", "{\"action\": \"wait\"}");
            json.endObjectInArray();
        }
    }
    
    // Nearby objects hint
    Object* nearestEnemy = NULL;
    Object* nearestItem = NULL;
    int nearestEnemyDist = 999;
    int nearestItemDist = 999;
    
    Object* obj = obj_find_first_at(obj_dude->elevation);
    while (obj != NULL) {
        if (obj != obj_dude) {
            int distance = obj_dist(obj_dude, obj);
            if (distance <= 10) {
                if (FID_TYPE(obj->fid) == OBJ_TYPE_CRITTER && !critter_is_dead(obj)) {
                    if (distance < nearestEnemyDist) {
                        nearestEnemy = obj;
                        nearestEnemyDist = distance;
                    }
                } else if (FID_TYPE(obj->fid) == OBJ_TYPE_ITEM) {
                    if (distance < nearestItemDist) {
                        nearestItem = obj;
                        nearestItemDist = distance;
                    }
                }
            }
        }
        obj = obj_find_next_at();
    }
    
    if (nearestEnemy && isInCombat()) {
        json.addObjectInArray();
        json.addString("priority", "HIGH");
        char hintText[128];
        snprintf(hintText, sizeof(hintText), "Enemy at distance %d. Consider attacking.", nearestEnemyDist);
        json.addString("hint", hintText);
        char actionText[128];
        snprintf(actionText, sizeof(actionText), "{\"action\": \"attack\", \"target_tile\": %d}", nearestEnemy->tile);
        json.addString("suggested_action", actionText);
        json.endObjectInArray();
    }
    
    if (nearestItem && !isInCombat()) {
        json.addObjectInArray();
        json.addString("priority", "LOW");
        char hintText[128];
        char* itemName = object_name(nearestItem);
        snprintf(hintText, sizeof(hintText), "Item '%s' nearby at distance %d.", itemName ? itemName : "Unknown", nearestItemDist);
        json.addString("hint", hintText);
        char actionText[128];
        snprintf(actionText, sizeof(actionText), "{\"action\": \"pickup\", \"target_tile\": %d, \"target_pid\": %d}", nearestItem->tile, nearestItem->pid);
        json.addString("suggested_action", actionText);
        json.endObjectInArray();
    }
    
    // No stimpaks warning
    bool hasStimpak = false;
    for (int i = 0; i < inventory->length; i++) {
        InventoryItem* item = &(inventory->items[i]);
        if (item->item && item->item->pid == 40) {
            hasStimpak = true;
            break;
        }
    }
    
    if (!hasStimpak && hpPercent < 100.0f) {
        json.addObjectInArray();
        json.addString("priority", "MEDIUM");
        json.addString("hint", "No Stimpaks in inventory! Look for healing items.");
        json.addString("info", "Search containers and bodies for Stimpaks (PID 40)");
        json.endObjectInArray();
    }
    
    json.endArray();
    
    // Help text - remind AI how to act
    json.addString("help", "Read this state. Check HP and hints. Decide action from: move, attack, use_item, pickup, wait. Write valid JSON to ai_action.json. Example: {\"action\": \"move\", \"target_tile\": 20150}");
    
    // Recent events (last 10)
    json.startArray("recent_events");
    int eventsToShow = gEventCount < MAX_RECENT_EVENTS ? gEventCount : MAX_RECENT_EVENTS;
    int startIdx = gEventCount < MAX_RECENT_EVENTS ? 0 : gEventWriteIndex;
    for (int i = 0; i < eventsToShow; i++) {
        int idx = (startIdx + i) % MAX_RECENT_EVENTS;
        if (gRecentEvents[idx][0] != '\0') {
            json.addObjectInArray();
            json.addString("event", gRecentEvents[idx]);
            json.endObjectInArray();
        }
    }
    json.endArray();
    
    json.endObject();
    
    // Write to temporary file first, then rename (atomic operation)
    char tempPath[256];
    snprintf(tempPath, sizeof(tempPath), "%s.tmp", kStateFilePath);
    
    FILE* fp = fopen(tempPath, "w");
    if (fp) {
        fwrite(json.c_str(), 1, json.length(), fp);
        fclose(fp);
        
        // Atomic rename
        rename(tempPath, kStateFilePath);
    }
}

// Simple JSON parser for reading action
static bool readAction(char* actionType, int* targetTile, int* targetPid) {
    FILE* fp = fopen(kActionFilePath, "r");
    if (!fp) {
        return false;
    }
    
    char buffer[4096];
    size_t bytesRead = fread(buffer, 1, sizeof(buffer) - 1, fp);
    fclose(fp);
    
    if (bytesRead == 0) {
        return false;
    }
    buffer[bytesRead] = '\0';
    
    // Simple parsing - look for "action":"value"
    char* actionStart = strstr(buffer, "\"action\"");
    if (actionStart) {
        char* valueStart = strchr(actionStart, ':');
        if (valueStart) {
            valueStart++; // skip :
            while (*valueStart == ' ' || *valueStart == '"') valueStart++;
            
            char* valueEnd = valueStart;
            while (*valueEnd && *valueEnd != '"' && *valueEnd != ',' && *valueEnd != '}') {
                valueEnd++;
            }
            
            size_t len = valueEnd - valueStart;
            if (len > 0 && len < 64) {
                strncpy(actionType, valueStart, len);
                actionType[len] = '\0';
            }
        }
    }
    
    // Look for "target_tile":value
    char* tileStart = strstr(buffer, "\"target_tile\"");
    if (tileStart && targetTile) {
        char* valueStart = strchr(tileStart, ':');
        if (valueStart) {
            *targetTile = atoi(valueStart + 1);
        }
    }
    
    // Look for "target_pid":value
    char* pidStart = strstr(buffer, "\"target_pid\"");
    if (pidStart && targetPid) {
        char* valueStart = strchr(pidStart, ':');
        if (valueStart) {
            *targetPid = atoi(valueStart + 1);
        }
    }
    
    // Delete the action file after reading
    remove(kActionFilePath);
    
    return actionType[0] != '\0';
}

// Execute the action parsed from JSON
static bool executeAction(const char* actionType, int targetTile, int targetPid) {
    if (!obj_dude || !actionType) {
        snprintf(gLastActionResult, sizeof(gLastActionResult), "error: invalid action");
        return false;
    }
    
    // Check cooldown
    int currentTime = getCurrentTimeMs();
    if (currentTime - gLastActionTimestamp < gActionCooldownMs) {
        snprintf(gLastActionResult, sizeof(gLastActionResult), "error: cooldown active");
        return false;
    }
    
    gLastActionTimestamp = currentTime;
    
    // Move to tile
    if (strcmp(actionType, "move") == 0) {
        if (targetTile >= 0 && targetTile < 40000) {
            if (!isInCombat()) {
                int result = obj_attempt_placement(obj_dude, targetTile, obj_dude->elevation, 0);
                if (result == 0) {
                    snprintf(gLastActionResult, sizeof(gLastActionResult), "success: moved to tile %d", targetTile);
                    addEvent("move", "Player moved");
                    addMemory("move", "exploration", "Moved to new location");
                } else {
                    snprintf(gLastActionResult, sizeof(gLastActionResult), "error: cannot move to tile %d", targetTile);
                    addMemory("move", "blocked", "Path blocked or invalid tile");
                }
            } else {
                int distance = tile_dist(obj_dude->tile, targetTile);
                int apCost = item_mp_cost(obj_dude, HIT_MODE_PUNCH, false);
                if (obj_dude->data.critter.combat.ap >= apCost) {
                    int result = obj_attempt_placement(obj_dude, targetTile, obj_dude->elevation, 0);
                    if (result == 0) {
                        obj_dude->data.critter.combat.ap -= apCost;
                        snprintf(gLastActionResult, sizeof(gLastActionResult), "success: moved to tile %d (-%d AP)", targetTile, apCost);
                        addEvent("move", "Player moved in combat");
                        addMemory("move_combat", "tactical positioning", "Repositioned during combat");
                    } else {
                        snprintf(gLastActionResult, sizeof(gLastActionResult), "error: cannot move to tile %d", targetTile);
                        addMemory("move_combat", "failed", "Could not reposition in combat");
                    }
                } else {
                    snprintf(gLastActionResult, sizeof(gLastActionResult), "error: not enough AP");
                    addMemory("move_combat", "insufficient AP", "Tried to move without enough action points");
                }
            }
        } else {
            snprintf(gLastActionResult, sizeof(gLastActionResult), "error: invalid tile %d", targetTile);
        }
        return true;
    }
    
    // Wait/skip turn
    if (strcmp(actionType, "wait") == 0) {
        if (isInCombat()) {
            combat_turn_run();
            snprintf(gLastActionResult, sizeof(gLastActionResult), "success: turn ended");
            addEvent("wait", "Turn skipped");
            addMemory("wait", "combat turn", "Ended combat turn");
        } else {
            snprintf(gLastActionResult, sizeof(gLastActionResult), "success: waited");
            addMemory("wait", "non-combat", "Waited/passed time");
        }
        return true;
    }
    
    // Use item
    if (strcmp(actionType, "use_item") == 0) {
        Inventory* inventory = &(obj_dude->data.critter.inventory);
        for (int i = 0; i < inventory->length; i++) {
            InventoryItem* invItem = &(inventory->items[i]);
            if (invItem->item && invItem->item->pid == targetPid) {
                obj_use_item(obj_dude, invItem->item);
                char* itemName = object_name(invItem->item);
                snprintf(gLastActionResult, sizeof(gLastActionResult), "success: used %s", itemName ? itemName : "item");
                addEvent("use_item", gLastActionResult + 9); // Skip "success: "
                addMemory("use_item", itemName ? itemName : "unknown item", gLastActionResult);
                return true;
            }
        }
        snprintf(gLastActionResult, sizeof(gLastActionResult), "error: item %d not found in inventory", targetPid);
        addMemory("use_item", "not found", gLastActionResult);
        return true;
    }
    
    // Pickup item
    if (strcmp(actionType, "pickup") == 0) {
        Object* obj = obj_find_first_at(obj_dude->elevation);
        while (obj != NULL) {
            if (obj->tile == targetTile && obj->pid == targetPid) {
                if (FID_TYPE(obj->fid) == OBJ_TYPE_ITEM) {
                    char* itemName = object_name(obj);
                    int result = obj_pickup(obj_dude, obj);
                    if (result == 0) {
                        snprintf(gLastActionResult, sizeof(gLastActionResult), "success: picked up %s", itemName ? itemName : "item");
                        addEvent("pickup", gLastActionResult + 9);
                        addMemory("pickup", itemName ? itemName : "item", "Successfully added to inventory");
                        // Track item collected
                        addItemCollected(targetPid, itemName, 1);
                        // Check for milestone items
                        if (targetPid == 40) { // Stimpak
                            addMilestone("Found first Stimpak - essential for survival");
                        }
                    } else {
                        snprintf(gLastActionResult, sizeof(gLastActionResult), "error: cannot pickup item");
                        addMemory("pickup", itemName ? itemName : "item", "Failed - inventory full or too heavy");
                    }
                    return true;
                }
            }
            obj = obj_find_next_at();
        }
        snprintf(gLastActionResult, sizeof(gLastActionResult), "error: item not found at tile %d", targetTile);
        addMemory("pickup", "not found", gLastActionResult);
        return true;
    }
    
    // Attack target
    if (strcmp(actionType, "attack") == 0) {
        Object* target = NULL;
        Object* obj = obj_find_first_at(obj_dude->elevation);
        while (obj != NULL) {
            if (obj->tile == targetTile && FID_TYPE(obj->fid) == OBJ_TYPE_CRITTER) {
                target = obj;
                break;
            }
            obj = obj_find_next_at();
        }
        
        if (target) {
            char* targetName = object_name(target);
            combat_attack(obj_dude, target, HIT_MODE_LEFT_WEAPON_PRIMARY, HIT_LOCATION_TORSO);
            snprintf(gLastActionResult, sizeof(gLastActionResult), "success: attacked %s", targetName ? targetName : "target");
            addEvent("attack", gLastActionResult + 9);
            
            char memoryResult[128];
            if (critter_is_dead(target)) {
                snprintf(memoryResult, sizeof(memoryResult), "Killed %s - threat eliminated", targetName ? targetName : "enemy");
                gTotalKills++;
            } else {
                snprintf(memoryResult, sizeof(memoryResult), "Attacked %s - still alive", targetName ? targetName : "enemy");
            }
            addMemory("attack", targetName ? targetName : "enemy", memoryResult);
            gTotalDamageDealt++; // Simplified tracking
            return true;
        } else {
            snprintf(gLastActionResult, sizeof(gLastActionResult), "error: no target at tile %d", targetTile);
            addMemory("attack", "no target", "Attempted attack but no enemy present");
        }
        return true;
    }
    
    snprintf(gLastActionResult, sizeof(gLastActionResult), "error: unknown action '%s'", actionType);
    return false;
}

// Initialize AI control API
void ai_control_api_init() {
    int value = 0;
    if (config_get_value(&game_config, GAME_CONFIG_PREFERENCES_KEY, "ai_control_api", &value)) {
        gAiControlApiEnabled = (value != 0);
    }
    
    if (gAiControlApiEnabled) {
        // Initialize tracking
        gSessionStartTime = getCurrentTimeMs();
        gLastHitPoints = 0;
        gLastLevel = 0;
        gTotalDamageDealt = 0;
        gTotalKills = 0;
        gEventCount = 0;
        gEventWriteIndex = 0;
        gKnowledgeWritten = false;
        gMemoryCount = 0;
        gMemoryIndex = 0;
        snprintf(gLastActionResult, sizeof(gLastActionResult), "none");
        
        // Clear events
        for (int i = 0; i < MAX_RECENT_EVENTS; i++) {
            gRecentEvents[i][0] = '\0';
        }
        
        // Clear memory
        for (int i = 0; i < MAX_MEMORY_ENTRIES; i++) {
            gMemoryEntries[i].active = false;
            gMemoryEntries[i].mapName[0] = '\0';
            gMemoryEntries[i].action[0] = '\0';
            gMemoryEntries[i].target[0] = '\0';
            gMemoryEntries[i].result[0] = '\0';
        }
        
        // Clear items collected
        for (int i = 0; i < MAX_ITEMS_COLLECTED; i++) {
            gItemsCollected[i].active = false;
            gItemsCollected[i].name[0] = '\0';
            gItemsCollected[i].mapName[0] = '\0';
        }
        
        // Clear milestones
        for (int i = 0; i < MAX_MILESTONES; i++) {
            gMilestones[i].active = false;
            gMilestones[i].description[0] = '\0';
            gMilestones[i].location[0] = '\0';
        }
        
        addEvent("system", "AI Control API initialized");
        addMilestone("Journey begins - Vault Dweller leaves Vault 13");
        
        // Write game knowledge base for AI
        writeGameKnowledge();
    }
}

// Cleanup AI control API
void ai_control_api_exit() {
    if (gAiControlApiEnabled) {
        addEvent("system", "AI Control API shutting down");
        // Final memory write
        writeMemory();
    }
    
    gAiControlApiEnabled = false;
    
    // Clean up any remaining files
    remove(kActionFilePath);
    remove(kStateFilePath);
    remove(kEventsFilePath);
    remove(kKnowledgeFilePath);
    // Keep memory file for analysis
}

// Check if API is enabled
bool ai_control_api_enabled() {
    return gAiControlApiEnabled;
}

// Export comprehensive character data for external website
static void writeCharacterData() {
    if (!obj_dude) return;
    
    JsonWriter json;
    json.startObject();
    
    // Meta information
    json.addString("data_type", "character_journey");
    json.addString("game", "Fallout 1");
    json.addInt("timestamp", getCurrentTimeMs() / 1000);
    json.addInt("session_time_seconds", (getCurrentTimeMs() - gSessionStartTime) / 1000);
    
    // Character basic info
    json.addInt("level", stat_pc_get(PC_STAT_LEVEL));
    json.addInt("experience", stat_pc_get(PC_STAT_EXPERIENCE));
    json.addInt("hit_points", critter_get_hits(obj_dude));
    json.addInt("max_hit_points", stat_level(obj_dude, STAT_MAXIMUM_HIT_POINTS));
    json.addInt("action_points", obj_dude->data.critter.combat.ap);
    json.addInt("max_action_points", stat_level(obj_dude, STAT_MAXIMUM_ACTION_POINTS));
    json.addInt("armor_class", stat_level(obj_dude, STAT_ARMOR_CLASS));
    json.addInt("sequence", stat_level(obj_dude, STAT_SEQUENCE));
    
    // Location
    int mapIndex = map_get_index_number();
    if (mapIndex != -1) {
        char* mapName = map_get_short_name(mapIndex);
        if (mapName) {
            json.addString("current_location", mapName);
        }
    }
    json.addInt("player_tile", obj_dude->tile);
    json.addInt("player_elevation", obj_dude->elevation);
    
    // Combat stats
    json.addBool("in_combat", isInCombat());
    json.addInt("total_kills", gTotalKills);
    json.addInt("total_damage_dealt", gTotalDamageDealt);
    
    // SPECIAL attributes
    json.startArray("special");
    const char* specialNames[] = {"Strength", "Perception", "Endurance", "Charisma", "Intelligence", "Agility", "Luck"};
    for (int i = 0; i < 7; i++) {
        json.addObjectInArray();
        json.addString("name", specialNames[i]);
        json.addInt("value", stat_level(obj_dude, i));
        json.endObjectInArray();
    }
    json.endArray();
    
    // Skills
    json.startArray("skills");
    const char* skillNames[] = {
        "Small Guns", "Big Guns", "Energy Weapons", "Unarmed", "Melee Weapons",
        "Throwing", "First Aid", "Doctor", "Sneak", "Lockpick", "Steal",
        "Traps", "Science", "Repair", "Speech", "Barter", "Gambling", "Outdoorsman"
    };
    for (int i = 0; i < 18; i++) {
        json.addObjectInArray();
        json.addString("name", skillNames[i]);
        json.addInt("value", skill_level(obj_dude, i));
        json.endObjectInArray();
    }
    json.endArray();
    
    // Perks
    json.startArray("perks");
    int perks[128];
    int perkCount = perk_make_list(perks);
    for (int i = 0; i < perkCount && i < 20; i++) {
        json.addObjectInArray();
        json.addString("name", perk_name(perks[i]));
        json.addInt("level", perk_level(perks[i]));
        json.endObjectInArray();
    }
    json.endArray();
    
    // Current inventory
    json.startArray("current_inventory");
    Inventory* inventory = &(obj_dude->data.critter.inventory);
    for (int i = 0; i < inventory->length && i < 50; i++) {
        InventoryItem* item = &(inventory->items[i]);
        if (item->item) {
            json.addObjectInArray();
            json.addInt("pid", item->item->pid);
            json.addString("name", object_name(item->item));
            json.addInt("quantity", item->quantity);
            json.endObjectInArray();
        }
    }
    json.endArray();
    
    // Items collected history
    json.startArray("items_collected");
    int startIdx = (gItemsCollectedCount >= MAX_ITEMS_COLLECTED) ? gItemsCollectedIndex : 0;
    for (int i = 0; i < gItemsCollectedCount && i < MAX_ITEMS_COLLECTED; i++) {
        int idx = (startIdx + i) % MAX_ITEMS_COLLECTED;
        if (gItemsCollected[idx].active) {
            json.addObjectInArray();
            json.addInt("pid", gItemsCollected[idx].pid);
            json.addString("name", gItemsCollected[idx].name);
            json.addInt("quantity", gItemsCollected[idx].quantity);
            json.addString("location", gItemsCollected[idx].mapName);
            json.addInt("timestamp", gItemsCollected[idx].timestamp);
            json.endObjectInArray();
        }
    }
    json.endArray();
    
    // Journey memories
    json.startArray("journey_memories");
    startIdx = (gMemoryCount >= MAX_MEMORY_ENTRIES) ? gMemoryIndex : 0;
    for (int i = 0; i < gMemoryCount && i < MAX_MEMORY_ENTRIES; i++) {
        int idx = (startIdx + i) % MAX_MEMORY_ENTRIES;
        if (gMemoryEntries[idx].active) {
            json.addObjectInArray();
            json.addString("map", gMemoryEntries[idx].mapName);
            json.addInt("tile", gMemoryEntries[idx].tile);
            json.addString("action", gMemoryEntries[idx].action);
            json.addString("target", gMemoryEntries[idx].target);
            json.addString("result", gMemoryEntries[idx].result);
            json.addInt("timestamp", gMemoryEntries[idx].timestamp);
            json.endObjectInArray();
        }
    }
    json.endArray();
    
    // Milestones
    json.startArray("milestones");
    startIdx = (gMilestonesCount >= MAX_MILESTONES) ? gMilestonesIndex : 0;
    for (int i = 0; i < gMilestonesCount && i < MAX_MILESTONES; i++) {
        int idx = (startIdx + i) % MAX_MILESTONES;
        if (gMilestones[idx].active) {
            json.addObjectInArray();
            json.addString("description", gMilestones[idx].description);
            json.addString("location", gMilestones[idx].location);
            json.addInt("timestamp", gMilestones[idx].timestamp);
            json.endObjectInArray();
        }
    }
    json.endArray();
    
    // Recent events
    json.startArray("recent_events");
    int eventsToShow = gEventCount < MAX_RECENT_EVENTS ? gEventCount : MAX_RECENT_EVENTS;
    startIdx = gEventCount < MAX_RECENT_EVENTS ? 0 : gEventWriteIndex;
    for (int i = 0; i < eventsToShow; i++) {
        int idx = (startIdx + i) % MAX_RECENT_EVENTS;
        if (gRecentEvents[idx][0] != '\0') {
            json.addObjectInArray();
            json.addString("event", gRecentEvents[idx]);
            json.endObjectInArray();
        }
    }
    json.endArray();
    
    json.endObject();
    
    // Write to file
    char tempPath[256];
    snprintf(tempPath, sizeof(tempPath), "character_data.json.tmp");
    
    FILE* fp = fopen(tempPath, "w");
    if (fp) {
        fwrite(json.c_str(), 1, json.length(), fp);
        fclose(fp);
        rename(tempPath, "character_data.json");
    }
}

// Process one AI action and write state
bool ai_control_api_process() {
    if (!gAiControlApiEnabled || !obj_dude) {
        return false;
    }
    
    // Always write current state
    writeGameState();
    
    // Write character data and memory periodically (every 10 frames to reduce overhead)
    static int frameCount = 0;
    frameCount++;
    if (frameCount >= 10) {
        writeMemory();
        writeCharacterData(); // Export data for external website
        frameCount = 0;
    }
    
    // Check for action file
    char actionType[64] = {0};
    int targetTile = -1;
    int targetPid = -1;
    
    if (readAction(actionType, &targetTile, &targetPid)) {
        return executeAction(actionType, targetTile, targetPid);
    }
    
    return false;
}

} // namespace fallout
