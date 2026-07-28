# Extended Character Data JSON Schema

This document describes the extended JSON schema for the Fallout 1 stream companion website. This schema extends the base game data exports with additional fields needed for the terminal UI features.

## Overview

The extended data provides comprehensive character information including:
- Character identity and visuals
- Complete stats and progression
- Interactive world map with location tracking
- Chronological timeline of events
- Quest log with outcomes
- In-character journal entries
- Faction relations and reputation

## JSON Structure

### Root Object

```json
{
  "character": { ... },      // Character identity
  "visuals": { ... },        // Visual assets
  "stats": { ... },          // Derived stats
  "special": { ... },        // SPECIAL attributes
  "skills": [ ... ],         // Skills array
  "perks": [ ... ],          // Perks array
  "traits": [ ... ],         // Traits array
  "inventory": { ... },      // Inventory with equipped/notable items
  "quests": [ ... ],         // Quest log
  "journal": [ ... ],        // Journal entries
  "relations": { ... },      // Faction relations
  "currentLocation": "...",  // Current map name
  "map": { ... },            // Map with locations and routes
  "locations": { ... },      // Detailed location dossiers
  "timeline": { ... },       // Chronological timeline
  "streamHighlights": [ ... ] // Quick recap bullets
}
```

## Detailed Field Descriptions

### character

Character identity and background information.

```json
{
  "name": "Jack Morrison",
  "age": 28,
  "pronouns": "he/him",
  "origin": "Vault 13",
  "background": "Vault Dweller",
  "tagline": "Chosen to find the water chip. The fate of Vault 13 rests on my shoulders."
}
```

### visuals

References to visual assets for display.

```json
{
  "portraitUrl": "/assets/portrait.png",  // Character portrait
  "spriteUrl": "/assets/sprite.gif",      // In-game sprite
  "themeColor": "#0f0"                    // Primary UI color hint
}
```

### stats

Derived statistics and progression.

```json
{
  "level": 5,
  "experience": 4250,
  "hp": 42,
  "maxHp": 55,
  "ap": 8,
  "maxAp": 10,
  "ac": 15,
  "sequence": 12,
  "healingRate": 2,
  "criticalChance": 5
}
```

### special

S.P.E.C.I.A.L. attributes (1-10 scale).

```json
{
  "strength": 6,
  "perception": 7,
  "endurance": 5,
  "charisma": 4,
  "intelligence": 8,
  "agility": 7,
  "luck": 5
}
```

### skills

Array of skills with values and optional tags.

```json
[
  {
    "name": "Small Guns",
    "value": 65,
    "tag": "primary"     // Optional: "primary", "secondary", or omitted
  },
  {
    "name": "Science",
    "value": 58,
    "tag": "primary"
  }
]
```

**Tags:**
- `"primary"` - High-level skills (≥50%)
- `"secondary"` - Mid-level skills (35-49%)
- No tag - Other skills

### perks

Array of perks with ranks and descriptions.

```json
[
  {
    "name": "Bonus Move",
    "rank": 1,
    "description": "+2 Action Points"
  }
]
```

### traits

Array of character traits (set at character creation).

```json
[
  {
    "name": "Gifted",
    "description": "+1 to all SPECIAL, -10% to all Skills"
  }
]
```

### inventory

Organized inventory with equipped and notable items.

```json
{
  "equipped": [
    {
      "slot": "Weapon",
      "name": "10mm Pistol",
      "pid": 8
    },
    {
      "slot": "Armor",
      "name": "Leather Armor",
      "pid": 74
    }
  ],
  "notable": [
    {
      "name": "Stimpak",
      "quantity": 8,
      "pid": 40,
      "note": "Essential healing item"
    }
  ]
}
```

### quests

Quest log with status tracking and outcomes.

```json
[
  {
    "id": "waterchip",
    "name": "Find the Water Chip",
    "status": "active",              // "active", "completed", "failed"
    "highlight": true,               // Featured quest
    "description": "The vault water chip is broken. Find a replacement within 150 days.",
    "outcome": "Success - ...",      // Present for completed quests
    "linkedLocations": ["vault13", "shady-sands"]  // Related location IDs
  }
]
```

### journal

In-character journal entries with dates and tags.

```json
[
  {
    "date": "Day 45",
    "entry": "The wasteland is harsher than I ever imagined...",
    "tags": ["shady-sands", "wasteland"]
  }
]
```

### relations

Faction reputation and overall karma.

```json
{
  "karma": "Good (250)",
  "factions": [
    {
      "name": "Shady Sands",
      "reputation": "Idolized",      // Text representation
      "standing": 95                 // Numeric value (-100 to +100)
    }
  ]
}
```

**Reputation Levels:**
- `"Idolized"` (75-100)
- `"Liked"` (25-74)
- `"Neutral"` (-24 to 24)
- `"Disliked"` (-25 to -74)
- `"Hated"` (-75 to -100)
- `"Unknown"` (not encountered)

### map

Interactive map data with locations and travel route.

```json
{
  "mapImage": "/assets/fallout1-map.png",
  "locations": [
    {
      "id": "vault13",
      "name": "Vault 13",
      "x": 30,                       // Map X coordinate (percentage)
      "y": 40,                       // Map Y coordinate (percentage)
      "visited": true,
      "type": "vault"                // "vault", "settlement", "city", "ruins", "hostile"
    }
  ],
  "route": [
    {
      "locationId": "vault13",
      "timestamp": "Day 1",
      "order": 1
    }
  ]
}
```

### locations

Detailed location dossiers keyed by location ID.

```json
{
  "vault13": {
    "id": "vault13",
    "name": "Vault 13",
    "summary": "Home. An underground sanctuary built by Vault-Tec...",
    "firstArrival": "Day 1 - Born here",
    "visited": true,
    "events": [
      {
        "id": "e1",
        "title": "Chosen as the Vault Dweller",
        "description": "Selected by the Overseer to venture into the wasteland...",
        "linkedQuestId": "waterchip",  // Optional
        "order": 1
      }
    ],
    "npcs": [
      {
        "name": "Overseer",
        "note": "Leader of Vault 13. Gave me the mission."
      }
    ],
    "tags": ["story-critical", "vault", "safe"],
    "consequences": {
      "karma": 0,
      "reputation": {
        "Vault 13": +10
      }
    }
  }
}
```

**Location Tags:**
- `"story-critical"` / `"story-location"` - Major story locations
- `"combat-heavy"` - Dangerous combat areas
- `"stealth-heavy"` - Stealth-focused locations
- `"diplomacy-heavy"` - Dialogue/persuasion focus
- `"faction-related"` - Faction headquarters/territory
- `"settlement"` / `"vault"` / `"hostile"` / `"ruins"` - Location types
- `"safe"` / `"dangerous"` - Safety level
- `"cleared"` - Threats eliminated

### timeline

Chronological event feed merging locations, quests, journal, and combat.

```json
{
  "entries": [
    {
      "id": "t1",
      "type": "quest",               // "quest", "location", "combat", "journal", "milestone"
      "date": "Day 1",
      "order": 1,
      "title": "Mission Received: Find the Water Chip",
      "shortSummary": "The Overseer chose me to save Vault 13...",
      "links": {
        "questId": "waterchip",      // Optional
        "locationId": "vault13",     // Optional
        "journalId": 0              // Optional journal entry index
      }
    }
  ]
}
```

**Timeline Entry Types:**
- `"quest"` - Quest milestones
- `"location"` - Location discoveries
- `"combat"` - Major battles
- `"journal"` - Journal entry moments
- `"milestone"` - Important achievements

### streamHighlights

Quick recap bullets for stream overlays (5-10 items).

```json
[
  "Currently in Junktown seeking water chip leads",
  "Level 5 Vault Dweller, specializing in Small Guns and Science",
  "105 days remaining to save Vault 13",
  "Idolized reputation with Shady Sands"
]
```

## Data Adapters & Backward Compatibility

The frontend is designed to work with graceful fallback:

1. **Missing Fields**: If optional fields are missing, UI shows "Unknown" or hides sections
2. **Empty Arrays**: Empty arrays are handled gracefully (e.g., no quests = "No active quests")
3. **Minimal Data**: System works with just basic game state (HP, location, skills)

### Adapter Pattern

```javascript
// Frontend adapter example
function adaptCharacterData(data) {
  return {
    character: data.character || { name: 'Vault Dweller', origin: 'Vault 13' },
    stats: data.stats || { level: 1, hp: 50, maxHp: 50 },
    special: data.special || { strength: 5, perception: 5, /* etc */ },
    skills: data.skills || [],
    // ... with defaults for all fields
  }
}
```

## Migration from Existing Data

If you have existing `ai_state.json` and `character_data.json` files:

1. **Run Generator**: Use `character_data_generator.py` to create extended data
2. **Automatic Parsing**: Generator reads existing JSON and extends it
3. **Incremental Updates**: New fields added as data becomes available

### Command

```bash
cd website/backend
python character_data_generator.py --game-dir ../.. --output ../../character_extended.json
```

## API Endpoints

Backend serves this data via REST API:

- `GET /api/character-extended` - Full extended character data
- `GET /api/timeline` - Timeline entries only
- `GET /api/quests` - Quest log only  
- `GET /api/locations-extended` - Locations and map data
- `GET /api/journal` - Journal entries only

## Best Practices

1. **Update Frequency**: Generate extended data every 1-10 seconds
2. **Caching**: Cache generated JSON to avoid regenerating on every request
3. **Incremental**: Build timeline/locations incrementally from game events
4. **Stream-Friendly**: Keep summaries concise (5-10 lines max)
5. **Tone**: Use Vault-Tec bureaucratic humor in descriptions

## Example Usage

### Generate Extended Data

```python
from character_data_generator import CharacterDataGenerator

generator = CharacterDataGenerator("../../")
extended_data = generator.generate_extended_data()
generator.save_extended_data("character_extended.json")
```

### Frontend Consumption

```javascript
// Fetch extended data
const response = await fetch('/api/character-extended')
const data = await response.json()

// Use in terminal UI
terminal.displayPanel('dossier', data)
```

## See Also

- [AI_CONTROL_API.md](../../AI_CONTROL_API.md) - Base game data format
- [Frontend README](../frontend/README.md) - UI implementation
- [Backend README](./README.md) - API server details
