# Quest Database Documentation

## Overview

The `quest_database.py` module provides comprehensive quest information for Fallout 1, mapping game GVAR (global variable) values to quest names, descriptions, objectives, and outcomes sourced from the Fallout wiki.

## Purpose

The game exports quest progress as numeric GVAR values (e.g., `GVAR_RESCUE_TANDI = 2`), but these numbers don't include quest names, descriptions, or objectives. This database bridges that gap by providing:

- **Quest names** from the Fallout wiki
- **Detailed descriptions** of each quest
- **Objectives** - step-by-step quest goals
- **Linked locations** - where quests take place
- **Rewards** - XP, items, reputation gains
- **Status interpretation** - what numeric GVAR values mean
- **Wiki URLs** - links to full quest guides

## Data Source

All quest information is sourced from:
- **Fallout Wiki**: https://fallout.fandom.com/wiki/Fallout_quests
- **Game scripts**: Quest GVAR definitions from game source code
- **Community knowledge**: Quest outcomes and branching paths

## Database Coverage

### Main Quests (3)
- Find the Water Chip
- Destroy the Mutant Vats
- Destroy the Master

### Critical Timers (2)
- Vault 13 Water Supply
- Days Until Vault 13 Discovered

### Major Side Quests (18)

**Shady Sands:**
- Rescue Tandi from the Raiders
- Stop the Radscorpions

**Junktown:**
- Junktown Power Struggle (Killian vs Gizmo)
- Bust the Skulz Gang

**The Hub:**
- Stop Decker's Conspiracy
- Find the Missing Caravans
- Join the Thieves' Guild

**Brotherhood of Steel:**
- Join the Brotherhood (Initiation)

**Necropolis:**
- Fix the Necropolis Water Pump

**Boneyard:**
- Help the Blades
- Help the Followers of the Apocalypse
- Free Adytum from the Regulators

**Special:**
- Kill the Deathclaw
- Acquire the Chryslus Highwayman (car)
- Build the Bridge to Arroyo

**Total: 27 quests tracked**

## Usage

### Basic Quest Lookup

```python
from quest_database import get_quest_info

# Get quest info from GVAR
quest = get_quest_info("GVAR_RESCUE_TANDI", 2)

print(quest['name'])          # "Rescue Tandi from the Raiders"
print(quest['status'])        # "completed"
print(quest['outcome'])       # "Tandi rescued and returned to Shady Sands"
print(quest['wiki_url'])      # Link to Fallout wiki
```

### Get All Quests

```python
from quest_database import get_all_quests

# Pass quest GVARs from game export
quest_gvars = {
    "GVAR_FIND_WATER_CHIP": 1,
    "GVAR_RESCUE_TANDI": 2,
    "GVAR_VAULT_WATER": 120
}

all_quests = get_all_quests(quest_gvars)

# Organized by status
active_quests = all_quests['active']
completed_quests = all_quests['completed']
failed_quests = all_quests['failed']
timers = all_quests['timers']
```

### Get Stream Highlights

```python
from quest_database import get_quest_highlights

# Get top 5 important quests for stream display
highlights = get_quest_highlights(quest_gvars)

for quest in highlights:
    print(f"{quest['name']} - {quest['status']}")
```

## Quest Status Interpretation

Each quest has custom status interpretation based on GVAR values:

### Standard Pattern
- `0` = Not started
- `1` = Active/In progress
- `2` = Completed
- `<0` = Failed

### Custom Patterns

Some quests have unique value meanings:

**GVAR_KILL_KILLIAN** (Junktown conflict):
- `0` = Not started
- `1` = Active
- `2` = Sided with Killian (Gizmo defeated)
- `3` = Sided with Gizmo (Killian killed)

**GVAR_FIX_NECROPOLIS** (Necropolis water):
- `0` = Not started
- `1` = Active
- `2` = Water pump repaired
- `3` = Water chip stolen

### Timer Quests

Timer quests (water supply, discovery countdown) interpret the GVAR value as days remaining:

```python
quest = get_quest_info("GVAR_VAULT_WATER", 95)
print(quest['days_remaining'])  # 95
print(quest['status'])          # "timer"
```

## Quest Data Structure

Each quest in the database contains:

```python
{
    "id": "rescue_tandi",                    # Unique identifier
    "name": "Rescue Tandi from the Raiders", # Display name
    "category": "side_quest",                # main_quest, side_quest, timer, achievement
    "location": "Shady Sands",               # Primary location
    "description": "...",                    # Full description
    "objectives": [                          # Step-by-step goals
        "Talk to Aradesh",
        "Locate raider camp",
        "Rescue Tandi",
        "Return to Shady Sands"
    ],
    "linked_locations": [                    # Related locations
        "Shady Sands",
        "Raider Camp"
    ],
    "rewards": "500 XP, reputation",         # Quest rewards
    "wiki_url": "https://...",               # Full guide link
    "status_values": {                       # GVAR value meanings
        0: {"status": "not_started", ...},
        1: {"status": "active", ...},
        2: {"status": "completed", "outcome": "...", ...}
    }
}
```

## Integration with Character Data Generator

The `character_data_generator.py` automatically uses the quest database:

```python
from character_data_generator import CharacterDataGenerator

generator = CharacterDataGenerator("../../")
extended_data = generator.generate_extended_data()

# Quest log now includes full wiki information
quests = extended_data['quests']

for quest in quests:
    print(f"{quest['name']} - {quest['status']}")
    print(f"  {quest['description']}")
    print(f"  Wiki: {quest['wiki_url']}")
```

## Frontend Display

The frontend can now display rich quest information:

```javascript
// Quest panel shows:
{
  "id": "rescue_tandi",
  "name": "Rescue Tandi from the Raiders",
  "status": "completed",
  "category": "Side Quest",
  "description": "Tandi, daughter of Aradesh...",
  "objectives": ["Talk to Aradesh", ...],
  "outcome": "Tandi rescued and returned to Shady Sands",
  "rewards": "500 XP, Aradesh's gratitude...",
  "linkedLocations": ["Shady Sands", "Raider Camp"],
  "wiki_url": "https://fallout.fandom.com/wiki/...",
  "progress": 100,
  "highlight": false,
  "source": "wiki"
}
```

## Adding New Quests

To add a new quest to the database:

1. Find the quest GVAR ID in `src/game/game_vars.h`
2. Research the quest on the Fallout wiki
3. Add entry to `QUEST_DATABASE` in `quest_database.py`:

```python
"GVAR_NEW_QUEST": {
    "id": "new_quest_id",
    "name": "Quest Name",
    "category": "side_quest",
    "location": "Location Name",
    "description": "Full description from wiki",
    "objectives": ["Step 1", "Step 2", ...],
    "linked_locations": ["Location1", "Location2"],
    "rewards": "XP and items",
    "wiki_url": "https://fallout.fandom.com/wiki/...",
    "status_values": {
        0: {"status": "not_started", "outcome": None, "progress": 0},
        1: {"status": "active", "outcome": "In progress", "progress": 50},
        2: {"status": "completed", "outcome": "Success", "progress": 100}
    }
}
```

## Benefits

### For Players/Viewers

- **Clear quest names** instead of "GVAR_123 = 2"
- **Full quest context** with descriptions and objectives
- **Progress tracking** with percentage completion
- **Quick wiki access** for full quest guides
- **Stream-friendly** display with highlights

### For Streamers

- **"Previously On..."** recap shows quest names and status
- **Timeline integration** with quest milestones
- **Location connections** show which quests link to locations
- **Quest highlights** display top 5 active quests automatically

### For Developers

- **Centralized quest data** - single source of truth
- **Easy maintenance** - add quests without touching frontend
- **Extensible** - add new fields (icons, difficulty, etc.)
- **Type-safe** - Python type hints for all functions

## Future Enhancements

Potential additions to the quest database:

1. **Quest icons** - Visual icons for each quest type
2. **Difficulty ratings** - Combat/stealth/diplomacy tags
3. **Quest chains** - Link related quests together
4. **NPC portraits** - Images of quest givers
5. **Map markers** - Coordinates for quest locations
6. **Branching paths** - Full outcome tree visualization
7. **Companion quests** - Quests requiring specific companions

## Performance

- **Zero runtime cost** - Database is Python dict lookups (O(1))
- **No external requests** - All data embedded in code
- **Minimal memory** - ~27 quest entries, ~100KB total
- **Fast serialization** - Ready for JSON API responses

## Maintenance

The quest database should be updated when:

1. **New quests discovered** - Community finds undocumented quests
2. **Wiki updates** - Quest guides are improved or corrected
3. **Game exports extended** - New quest GVARs added to game code
4. **Status values clarified** - Better understanding of GVAR meanings

## References

- Fallout Wiki Quest List: https://fallout.fandom.com/wiki/Fallout_quests
- Fallout Wiki Locations: https://fallout.fandom.com/wiki/Fallout_locations
- Game Source: `src/game/game_vars.h` (GVAR definitions)
- Quest Scripts: Various `.int` script files in game data

## License

Quest information sourced from the Fallout Wiki (CC-BY-SA 3.0) and game analysis.
