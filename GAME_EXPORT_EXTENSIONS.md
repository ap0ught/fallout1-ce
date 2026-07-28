# Game Export Extensions Proposal

## Goal
Extend the Fallout 1 C++ game code to export additional data fields that eliminate the need for backend generation, providing complete data directly from the game.

## Current State

### Already Exported by Game (ai_state.json)
✅ HP, Max HP, AP, Max AP
✅ Level, Experience  
✅ SPECIAL stats (all 7)
✅ Skills (all 18 with values)
✅ Perks (name, level/rank)
✅ Inventory (items with PID, name, quantity)
✅ Current location (map_name)
✅ Combat status
✅ Session stats (kills, damage, time)
✅ Recent events
✅ Nearby objects/NPCs

### NOT Currently Exported
❌ Character name
❌ Character age, gender
❌ Character pronouns (custom field)
❌ Traits (selected at character creation)
❌ Karma value
❌ Reputation value
❌ Town reputation (per-faction)
❌ Equipped weapon/armor slots
❌ Quest IDs and status
❌ Location visit timestamps
❌ Map coordinates for locations

## Proposed Extensions to ai_state.json

### 1. Character Identity (HIGH PRIORITY)
Add to `writeGameState()` function:

```cpp
// Character identity
const char* characterName = getCharacterName(); // To be implemented
if (characterName) {
    json.addString("character_name", characterName);
}

json.addInt("age", stat_level(obj_dude, STAT_AGE));
json.addInt("gender", stat_level(obj_dude, STAT_GENDER)); // 0=male, 1=female

// Custom field - could store in game config or character data
json.addString("pronouns", "they/them"); // Default, could be configurable
```

**Implementation Notes:**
- Character name is set during character creation
- Age and gender are already available via STAT_AGE and STAT_GENDER
- Pronouns could be added as a new character field or config option

### 2. Karma & Reputation (HIGH PRIORITY)
Add to `writeGameState()` function:

```cpp
// Karma and reputation
json.addInt("karma", stat_pc_get(PC_STAT_KARMA));
json.addInt("reputation", stat_pc_get(PC_STAT_REPUTATION));

// Town reputation (if available in game)
json.startArray("town_reputation");
// Loop through known towns/factions
const char* towns[] = {"Shady Sands", "Junktown", "Hub", "Brotherhood", "Necropolis", "Boneyard"};
for (int i = 0; i < 6; i++) {
    int rep = getTownReputation(i); // To be implemented - check if this API exists
    if (rep != 0) { // Only export if known
        json.addObjectInArray();
        json.addString("name", towns[i]);
        json.addInt("value", rep);
        json.endObjectInArray();
    }
}
json.endArray();
```

**Implementation Notes:**
- PC_STAT_KARMA and PC_STAT_REPUTATION already exist in stat_defs.h
- Need to check if town reputation is tracked in-game (likely is for dialogue/reactions)
- May need to locate the reputation system code

### 3. Character Traits (MEDIUM PRIORITY)
Add to `writeGameState()` function:

```cpp
// Character traits
json.startArray("traits");
int trait1, trait2;
trait_get(&trait1, &trait2); // Function exists in trait.h

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
```

**Implementation Notes:**
- `trait_get()`, `trait_name()`, and `trait_description()` already exist in trait.h
- This is straightforward to implement

### 4. Equipped Items (MEDIUM PRIORITY)
Add to `writeGameState()` function:

```cpp
// Equipped items
json.startObject("equipped");

// Weapon slots
Object* rightHand = inven_right_hand(obj_dude);
if (rightHand) {
    json.addObjectInField("right_hand");
    json.addInt("pid", rightHand->pid);
    json.addString("name", object_name(rightHand));
    json.endObject();
}

Object* leftHand = inven_left_hand(obj_dude);
if (leftHand) {
    json.addObjectInField("left_hand");
    json.addInt("pid", leftHand->pid);
    json.addString("name", object_name(leftHand));
    json.endObject();
}

// Armor
Object* armor = inven_worn(obj_dude);
if (armor) {
    json.addObjectInField("armor");
    json.addInt("pid", armor->pid);
    json.addString("name", object_name(armor));
    json.endObject();
}

json.endObject(); // equipped
```

**Implementation Notes:**
- `inven_right_hand()`, `inven_left_hand()`, `inven_worn()` likely exist in inventry.h
- Need to verify exact function names

### 5. Additional Derived Stats (LOW PRIORITY)
Add to `writeGameState()` function:

```cpp
// Additional derived stats
json.addInt("healing_rate", stat_level(obj_dude, STAT_HEALING_RATE));
json.addInt("critical_chance", stat_level(obj_dude, STAT_CRITICAL_CHANCE));
json.addInt("damage_resistance", stat_level(obj_dude, STAT_DAMAGE_RESISTANCE));
json.addInt("radiation_resistance", stat_level(obj_dude, STAT_RADIATION_RESISTANCE));
json.addInt("poison_resistance", stat_level(obj_dude, STAT_POISON_RESISTANCE));
json.addInt("current_radiation", stat_level(obj_dude, STAT_CURRENT_RADIATION_LEVEL));
json.addInt("current_poison", stat_level(obj_dude, STAT_CURRENT_POISON_LEVEL));
```

**Implementation Notes:**
- All these stats already exist in STAT enum
- Simple addition

## Quest Tracking Extension (OPTIONAL - MORE COMPLEX)

### 6. Quest System Export
This would require deeper integration if the game tracks quests. Create new file `ai_quests.json`:

```cpp
static void writeQuestData() {
    JsonWriter json;
    json.startObject();
    
    json.startArray("quests");
    
    // Main quest
    json.addObjectInArray();
    json.addString("id", "waterchip");
    json.addString("name", "Find the Water Chip");
    json.addString("status", getWaterChipQuestStatus()); // "active" or "completed"
    json.addInt("days_remaining", getDaysRemaining());
    json.endObjectInArray();
    
    // Would need to add quest tracking for side quests
    // This requires identifying how quests are tracked in game scripts
    
    json.endArray();
    json.endObject();
    
    writeJsonToFile("ai_quests.json", json);
}
```

**Implementation Notes:**
- This is more complex and requires understanding the game's quest/script system
- May require adding quest tracking hooks throughout the codebase
- Lower priority - can be inferred from memory/events

## Location Visit Tracking Extension (OPTIONAL)

### 7. Location Tracking
Add to memory system or create new tracking:

```cpp
// In existing memory or new location tracking
struct LocationVisit {
    char mapName[32];
    int firstVisitTimestamp;
    int visitCount;
};

// Track in memory when player enters new map
// Export in ai_state.json or separate ai_locations.json
```

**Implementation Notes:**
- Can piggyback on existing map_name tracking
- Add timestamp when map_name changes
- Medium complexity

## Implementation Priority

### Phase 1: Quick Wins (1-2 hours) ✅ COMPLETED
1. ✅ Karma and reputation (PC_STAT_KARMA, PC_STAT_REPUTATION)
2. ✅ Character traits (trait_get(), trait_name(), trait_description())
3. ✅ Age and gender (STAT_AGE, STAT_GENDER)
4. ✅ Additional derived stats (healing_rate, critical_chance, etc.)

### Phase 2: Character Identity (2-3 hours) ✅ COMPLETED
5. ✅ Character name (object_name(obj_dude))
6. ✅ Equipped items (inven_right_hand(), inven_left_hand(), inven_worn())

### Phase 3: Quest & Reputation Tracking (3-4 hours) ✅ COMPLETED
7. ✅ Quest tracking (27 major quests via game_get_global_var())
   - Main story: water chip, destroy vats, destroy master
   - Major side quests: rescue Tandi, kill deathclaw, Brotherhood initiate, etc.
8. ✅ Town reputation (hostile status for 12 factions via GVAR_ENEMY_*)
9. ✅ Location discovery (known/visited status for 12+ major locations)
10. ✅ Player worldmap location ID (GVAR_PLAYER_LOCATION)

### Phase 4: Advanced Features (6-10 hours) - FUTURE
11. ❌ Individual quest names/descriptions (requires string table lookup)
12. ❌ Quest outcomes/rewards (complex, requires interpretation of GVAR patterns)
13. ❌ Visit timestamps (would require new tracking system)
14. ❌ Detailed faction standings beyond hostile/neutral (may not exist in game)

## Testing Plan

After each phase:
1. Rebuild game with extensions
2. Run game and verify `ai_state.json` contains new fields
3. Test backend `character_data_generator.py` reads new fields
4. Update frontend to use direct fields instead of generated ones
5. Verify terminal UI displays correct data

## Files to Modify

### src/game/ai_control_api.cc
- `writeGameState()` function - add new JSON fields
- Add includes for reputation/town systems if needed

### src/game/ai_control_api.h
- No changes needed (internal functions)

### Configuration
- Potentially add `fallout.cfg` options for custom fields like pronouns

## Benefits

✅ **Accurate data** - Direct from game, no inference
✅ **Real-time updates** - Changes immediately reflected
✅ **Less backend processing** - No need to generate/infer data
✅ **Better debugging** - All data visible in exports
✅ **More features** - Unlock reputation, quests, etc.

## Backward Compatibility

All additions are **backward compatible**:
- Frontend has graceful fallback for missing fields
- `character_data_generator.py` still works if fields missing
- New fields are additions, no breaking changes

## Documentation Updates

After implementation:
1. Update `AI_CONTROL_API.md` with new fields
2. Update `JSON_COVERAGE_ANALYSIS.md` to show what's now covered
3. Update frontend README to note direct fields vs generated
4. Add examples of new JSON structure

## Success Criteria

✅ Karma and reputation appear in `ai_state.json`
✅ Traits appear in `ai_state.json`
✅ Character name appears in `ai_state.json`
✅ Equipped items (right_hand, left_hand, armor) appear in `ai_state.json`
✅ 27 major quest GVARs exported in `quests` object
✅ 12 faction hostile statuses exported in `town_reputation` object
✅ 12+ location discovery statuses exported in `locations_known` object
✅ Player worldmap location ID exported
✅ Frontend displays quest progress from game data
✅ Frontend displays faction relationships without backend inference
✅ ID card shows correct karma from game
✅ No frontend errors with new fields
✅ Backend generator uses direct fields when available

## Next Steps

1. **Review this proposal** - Ensure priorities align with needs
2. **Locate code** - Find reputation system, character name storage
3. **Implement Phase 1** - Quick wins (karma, traits, stats)
4. **Test** - Verify exports work correctly
5. **Implement Phase 2** - Character identity
6. **Update docs** - Keep documentation in sync
