# AI Control API - Complete Solution for AI-Driven Fallout 1 Streaming

## Overview

The AI Control API provides a comprehensive JSON-based interface for external AI agents (especially LLMs) to play Fallout 1 Community Edition with full character immersion, memory tracking, and automatic website generation for Twitch streaming. **Perfect for AI-controlled gameplay streams with full story tracking.**

## Configuration

To enable the API, add the following line to your `fallout.cfg` file under the `[preferences]` section:

```ini
ai_control_api=1
```

## Complete System Architecture

The API provides **everything needed for an AI-driven Twitch stream**:

1. **`ai_knowledge.json`** - Complete game encyclopedia (written once at startup)
2. **`ai_state.json`** - Live game state (updated every frame)
3. **`ai_action.json`** - AI writes actions here (read and deleted by game)
4. **`ai_memory.json`** - Decision memory log (persistent learning)
5. **`character_journey.html`** - Auto-generated website (updates every second)

## New: Character Role-Play & Memory System

### Role-Play Context (In ai_knowledge.json)

The AI receives complete character context to stay in character as the Vault Dweller:

- **Character Role**: "Vault Dweller from Vault 13"
- **Background**: Underground shelter resident, chosen to find water chip, 150 days to save vault
- **Personality**: Brave but inexperienced, cautious but determined, cares about vault survival
- **Speaking Style**: First-person, practical, survival-focused
- **Example**: "I need to find that water chip, but I should heal first - I'm badly injured."

The AI knows it's representing a real character with motivations, not just playing a game.

### Memory Tracking (ai_memory.json)

Every significant action is recorded with full context:

```json
{
  "description": "AI decision memory - records actions, outcomes, and learned experiences",
  "total_memories": 45,
  "memories": [
    {
      "map": "V13Ent",
      "tile": 20105,
      "elevation": 0,
      "action": "attack",
      "target": "Radscorpion",
      "result": "Killed Radscorpion - threat eliminated",
      "timestamp": 1704067200
    },
    {
      "map": "Junktown",
      "tile": 15200,
      "elevation": 0,
      "action": "use_item",
      "target": "Stimpak",
      "result": "success: used Stimpak",
      "timestamp": 1704067350
    }
  ]
}
```

**Memory enables:**
- Learning from past mistakes
- Remembering dangerous locations
- Tracking story progression
- Building strategy from experience
- Maintaining continuity across sessions

### Comprehensive Item & Enemy Database

The knowledge base now includes detailed stats:

**Weapons Database** (14 weapons):
- Name, damage range, AP cost, ammo type, tactical notes
- Example: "Plasma Rifle: 30-65 damage, 25 range, Microfusion ammo - Late game extremely powerful"

**Armor Database** (8 armor types):
- AC bonus, damage resistance, special properties
- Example: "Power Armor: +25 AC, 40% resist, +3 STR - Best armor, quest reward"

**Enemy Database** (10 enemy types):
- HP range, weaknesses, strengths, combat strategies
- Example: "Deathclaw: 200-300 HP, Weakness: Eye shots/plasma, DEADLY - Run if possible"

**Consumables** (16 items):
- PID, effects, tactical usage notes
- Example: "Stimpak (PID 40): Heals 15-20 HP - ESSENTIAL. Always carry 5+. Use at <50% HP"

**Ammunition Types** (9 types):
- Rarity, usage, availability notes

## Auto-Generated Website for Streaming

###  character_journey.html - Live Stream Overlay

The game automatically generates a beautiful HTML page that updates every 10 seconds, perfect for OBS browser source overlay:

**Features:**
- 🎮 **Real-time character stats** (HP, Level, XP, Location)
- ⭐ **S.P.E.C.I.A.L. attributes** with progress bars
- 🎯 **All 18 skills** with percentage bars
- 🌟 **Perks list** with ranks
- 📜 **Complete journey timeline** (last 50 memories)
- 📰 **Recent events feed**
- 🔄 **Auto-refresh** every 10 seconds
- 🎨 **Fallout-themed design** (green terminal aesthetic)

**Usage for Twitch:**
1. Open `character_journey.html` in OBS as Browser Source
2. Set width to 1920x1080 or your stream resolution
3. Enable "Refresh browser when scene becomes active"
4. Position as full-screen overlay or side panel
5. Chat can see live character progression!



## Enhanced Game State Format (`ai_state.json`)

The game writes extensive state information every frame:

```json
{
  "player_tile": 20100,
  "player_elevation": 0,
  "player_rotation": 2,
  "hit_points": 45,
  "max_hit_points": 45,
  "action_points": 7,
  "max_action_points": 10,
  "level": 3,
  "experience": 1250,
  "armor_class": 12,
  "sequence": 8,
  "carry_weight": 175,
  "melee_damage": 2,
  "healing_rate": 1,
  "critical_chance": 5,
  "damage_resistance": 0,
  "radiation_resistance": 10,
  "poison_resistance": 15,
  "karma": 25,
  "reputation": 50,
  "age": 25,
  "gender": 0,
  "character_name": "Jack Morrison",
  "in_combat": false,
  "strength": 5,
  "perception": 6,
  "endurance": 5,
  "charisma": 5,
  "intelligence": 6,
  "agility": 6,
  "luck": 5,
  "skills": [
    {"name": "Small Guns", "value": 45},
    {"name": "Big Guns", "value": 15},
    {"name": "Energy Weapons", "value": 20},
    {"name": "Unarmed", "value": 55},
    {"name": "Melee Weapons", "value": 40},
    {"name": "Throwing", "value": 30},
    {"name": "First Aid", "value": 35},
    {"name": "Doctor", "value": 25},
    {"name": "Sneak", "value": 40},
    {"name": "Lockpick", "value": 30},
    {"name": "Steal", "value": 25},
    {"name": "Traps", "value": 20},
    {"name": "Science", "value": 30},
    {"name": "Repair", "value": 25},
    {"name": "Speech", "value": 50},
    {"name": "Barter", "value": 35},
    {"name": "Gambling", "value": 20},
    {"name": "Outdoorsman", "value": 25}
  ],
  "perks": [
    {"name": "Bonus Move", "level": 1},
    {"name": "More Criticals", "level": 1}
  ],
  "traits": [
    {"name": "Gifted", "description": "+1 to all SPECIAL, -10% to all skills"},
    {"name": "Fast Shot", "description": "You can't use targeted shots, but gain +1 AP"}
  ],
  "map_name": "V13Ent",
  "nearby_objects": [
    {
      "tile": 20105,
      "distance": 2,
      "type": 1,
      "pid": 16777216,
      "name": "Vault Dweller",
      "is_dead": false,
      "hp": 30
    }
  ],
  "inventory": [
    {
      "pid": 41,
      "quantity": 350,
      "name": "$"
    },
    {
      "pid": 40,
      "quantity": 3,
      "name": "Stimpak"
    }
  ],
  "equipped": {
    "right_hand": {
      "pid": 7,
      "name": "Spear"
    },
    "armor": {
      "pid": 74,
      "name": "Leather Jacket"
    }
  },
  "quests": {
    "find_water_chip": 1,
    "destroy_vats": 0,
    "destroy_master": 0,
    "days_to_vault13_discovery": 500,
    "vault_water_days": 150,
    "rescue_tandi": 2,
    "tandi_status": 1,
    "kill_radscorpions": 1,
    "kill_deathclaw": 0,
    "capture_gizmo": 0,
    "kill_killian": 0,
    "missing_caravan": 0,
    "steal_necklace": 0,
    "become_an_initiate": 0,
    "find_lost_initiate": 0,
    "kill_merchant": 0,
    "kill_jain": 0,
    "kill_super_mutants": 0,
    "fix_necropolis_pump": 0,
    "necropolis_water_chip_taken": 0,
    "gang_war": 0,
    "destroy_followers": 0,
    "fix_farm": 0,
    "save_sinthia": 0,
    "cure_jarvis": 0,
    "make_antidote": 0
  },
  "town_reputation": {
    "vault_13": 0,
    "shady_sands": 0,
    "junktown": 0,
    "hub": 0,
    "necropolis": 0,
    "brotherhood": 0,
    "adytum": 0,
    "rippers": 0,
    "blades": 0,
    "raiders": 1,
    "cathedral": 0,
    "followers": 0
  },
  "locations_known": {
    "vault_13": 1,
    "vault_15": 1,
    "shady_sands": 1,
    "junktown": 0,
    "raiders": 1,
    "necropolis": 0,
    "hub": 0,
    "brotherhood": 0,
    "military_base": 0,
    "glow": 0,
    "boneyard": 0,
    "cathedral": 0,
    "necropolis_visited": 0,
    "necropolis_known": 0
  },
  "player_location_id": 0,
  "total_damage_dealt": 145,
  "total_kills": 3,
  "session_time_seconds": 1234,
  "last_action_result": "success: moved to tile 20150",
  "ai_hints": [
    {
      "priority": "HIGH",
      "hint": "HP below 50%. Consider healing soon.",
      "suggested_action": "{\"action\": \"use_item\", \"target_pid\": 40}"
    },
    {
      "priority": "INFO",
      "hint": "Item 'Stimpak' nearby at distance 3.",
      "suggested_action": "{\"action\": \"pickup\", \"target_tile\": 20108, \"target_pid\": 40}"
    }
  ],
  "help": "Read this state. Check HP and hints. Decide action from: move, attack, use_item, pickup, wait. Write valid JSON to ai_action.json.",
  "recent_events": [
    {"event": "damage_taken: Took 5 damage (HP: 50->45)"},
    {"event": "pickup: picked up Stimpak"},
    {"event": "attack: attacked Rat"},
    {"event": "move: Player moved"}
  ]
}
```

**New Fields Added (v2.1.0)**

**Derived Stats:**
- `healing_rate` - HP restored per rest/sleep
- `critical_chance` - Base critical hit chance percentage
- `damage_resistance` - General damage reduction
- `radiation_resistance` - Radiation damage reduction
- `poison_resistance` - Poison damage reduction

**Character Identity:**
- `karma` - Moral alignment value (-100 to +100, negative=evil, positive=good)
- `reputation` - Overall fame/recognition (-100 to +100)
- `age` - Character age in years
- `gender` - 0=male, 1=female
- `character_name` - Player character name (from character creation)

**Traits:**
- `traits[]` - Character traits selected at creation (up to 2)
  - Each trait has `name` and `description`
  - Traits provide permanent bonuses/penalties
  - Examples: Gifted, Fast Shot, Bloody Mess, Kamikaze, etc.

**Equipped Items:**
- `equipped` - Object with currently equipped items
  - `right_hand` - Weapon/item in right hand slot (pid, name)
  - `left_hand` - Weapon/item in left hand slot (pid, name)
  - `armor` - Currently worn armor (pid, name)

**New Fields Added (v2.2.0)**

**Quest Tracking:**
- `quests` - Object containing key quest global variables
  - Main story: `find_water_chip`, `destroy_vats`, `destroy_master`, `days_to_vault13_discovery`, `vault_water_days`
  - Major side quests: `rescue_tandi`, `kill_radscorpions`, `kill_deathclaw`, `capture_gizmo`, `become_an_initiate`, `fix_necropolis_pump`, etc.
  - Values indicate quest progress (0=not started, >0=in progress or completed, specific values vary by quest)
  - 27 major quests tracked

**Town Reputation:**
- `town_reputation` - Object with enemy status for each faction
  - Values: 0=neutral, 1=hostile
  - Tracks: Vault 13, Shady Sands, Junktown, Hub, Necropolis, Brotherhood, Adytum, Rippers, Blades, Raiders, Cathedral, Followers

**Location Discovery:**
- `locations_known` - Object with map discovery status
  - Values: 0=unknown, 1=known/discovered
  - Tracks: All 12 major towns plus special locations
  - Includes `necropolis_visited` and `necropolis_known` flags

**Player Location:**
- `player_location_id` - Current worldmap location ID (from GVAR_PLAYER_LOCATION)

### New: Real-Time AI Hints

The `ai_hints` array provides context-aware suggestions:
- **CRITICAL** - Immediate action needed (very low HP)
- **HIGH** - Important action recommended (low HP, enemy nearby)
- **MEDIUM** - Helpful suggestion (low AP, no stimpaks)
- **LOW** - Optional action (items to pick up)
- **INFO** - Informational context (combat mode active)

Each hint includes:
- `priority` - Urgency level
- `hint` - Human-readable description
- `suggested_action` - Ready-to-use JSON action (optional)
- `info` - Additional context (optional)

The `help` field provides a quick reminder of available actions and how to use them.

## Action Format (`ai_action.json`)

Actions now include result feedback and cooldown protection:

### Move Action
```json
{
  "action": "move",
  "target_tile": 20150
}
```
**Results:** `success: moved to tile X` or `error: cannot move to tile X` or `error: not enough AP`

### Wait/Skip Turn
```json
{
  "action": "wait"
}
```
**Results:** `success: turn ended` or `success: waited`

### Use Item
```json
{
  "action": "use_item",
  "target_pid": 40
}
```
**Results:** `success: used [item name]` or `error: item X not found in inventory`

### Pickup Item
```json
{
  "action": "pickup",
  "target_tile": 20105,
  "target_pid": 41
}
```
**Results:** `success: picked up [item name]` or `error: cannot pickup item` or `error: item not found at tile X`

### Attack Target
```json
{
  "action": "attack",
  "target_tile": 20105
}
```
**Results:** `success: attacked [target name]` or `error: no target at tile X`

## Streaming Features

### Event Tracking
The API automatically tracks important events:
- **Damage taken** - Player health changes
- **Level ups** - Character progression
- **Combat actions** - Attacks, kills
- **Item interactions** - Pickups, usage
- **Movement** - Location changes

### Streaming Stats
Perfect for OBS overlays:
- `total_damage_dealt` - Cumulative damage counter
- `total_kills` - Enemy kill counter
- `session_time_seconds` - Time since API started
- `recent_events` - Last 50 events for scrolling display

### Action Cooldown
100ms cooldown between actions prevents spam and ensures stability for Twitch chat integration.

## Twitch Integration Examples

### Python Twitch Bot
```python
import json
import time
import os
from twitchio.ext import commands

class FalloutBot(commands.Bot):
    def __init__(self):
        super().__init__(token='YOUR_TOKEN', prefix='!', initial_channels=['YOUR_CHANNEL'])
        self.action_votes = {}
        self.vote_end_time = 0
    
    def read_state(self):
        if os.path.exists("ai_state.json"):
            with open("ai_state.json", "r") as f:
                return json.load(f)
        return None
    
    def send_action(self, action, **kwargs):
        action_data = {"action": action}
        action_data.update(kwargs)
        with open("ai_action.json", "w") as f:
            json.dump(action_data, f)
    
    @commands.command(name='vote')
    async def vote(self, ctx, action: str):
        """Chat voting: !vote move or !vote attack or !vote wait"""
        if time.time() < self.vote_end_time:
            if action in ['move', 'attack', 'wait', 'heal']:
                self.action_votes[action] = self.action_votes.get(action, 0) + 1
                await ctx.send(f"Vote recorded for {action}!")
    
    @commands.command(name='startvote')
    async def start_vote(self, ctx):
        """Start a 30-second vote"""
        self.action_votes = {}
        self.vote_end_time = time.time() + 30
        await ctx.send("Vote now! Use !vote move, !vote attack, !vote wait, or !vote heal (30 seconds)")
    
    @commands.command(name='endvote')
    async def end_vote(self, ctx):
        """End vote and execute winning action"""
        if not self.action_votes:
            await ctx.send("No votes recorded!")
            return
        
        winner = max(self.action_votes, key=self.action_votes.get)
        await ctx.send(f"Winning action: {winner} with {self.action_votes[winner]} votes!")
        
        state = self.read_state()
        if state:
            if winner == 'move' and state['nearby_objects']:
                # Move to nearest object
                target = state['nearby_objects'][0]
                self.send_action("move", target_tile=target['tile'])
            elif winner == 'attack' and state['nearby_objects']:
                # Attack nearest enemy
                for obj in state['nearby_objects']:
                    if not obj.get('is_dead', True):
                        self.send_action("attack", target_tile=obj['tile'])
                        break
            elif winner == 'heal':
                # Use stimpak (pid 40)
                self.send_action("use_item", target_pid=40)
            elif winner == 'wait':
                self.send_action("wait")
        
        self.action_votes = {}
        self.vote_end_time = 0

bot = FalloutBot()
bot.run()
```

### Stream Overlay (HTML/JavaScript)
```html
<!DOCTYPE html>
<html>
<head>
    <style>
        body { 
            background: transparent; 
            font-family: 'Courier New', monospace;
            color: #00ff00;
        }
        .stats {
            background: rgba(0,0,0,0.8);
            padding: 10px;
            border: 2px solid #00ff00;
        }
    </style>
</head>
<body>
    <div class="stats" id="stats"></div>
    <script>
        async function updateStats() {
            const response = await fetch('ai_state.json');
            const state = await response.json();
            
            document.getElementById('stats').innerHTML = `
                <h2>FALLOUT 1 STATS</h2>
                <p>HP: ${state.hit_points}/${state.max_hit_points}</p>
                <p>Level: ${state.level} (XP: ${state.experience})</p>
                <p>Location: ${state.map_name}</p>
                <p>Session Time: ${Math.floor(state.session_time_seconds / 60)}m</p>
                <p>Kills: ${state.total_kills}</p>
                <p>Damage: ${state.total_damage_dealt}</p>
                <p>Last Action: ${state.last_action_result}</p>
                <h3>Recent Events:</h3>
                ${state.recent_events.map(e => `<p>• ${e.event}</p>`).join('')}
            `;
        }
        
        setInterval(updateStats, 1000); // Update every second
    </script>
</body>
</html>
```

## Implementation Details

- **Deterministic**: Actions are processed exactly once per frame
- **Low-overhead**: Minimal JSON parsing and writing
- **Safe**: Uses atomic file operations (write to temp, then rename)
- **Streaming-friendly**: Continuous state updates with event tracking
- **Cooldown protection**: 100ms minimum between actions
- **Action feedback**: Every action returns success/failure status
- **Event history**: Last 50 events tracked for overlays

## Best Practices for Twitch Streaming

1. **Use voting systems** - Let chat vote on actions every 30-60 seconds
2. **Display overlays** - Show stats, recent events, and action results
3. **Moderate cooldowns** - Don't spam actions too quickly
4. **Track achievements** - Use kill count, damage, and time for milestones
5. **Enable interaction** - Let viewers influence gameplay meaningfully

## Using the Knowledge Base for AI Models

### For AI Model Integration

The `ai_knowledge.json` file is designed to be read once at initialization to give an AI model complete understanding:

```python
import json

# Load knowledge base once at startup
with open('ai_knowledge.json', 'r') as f:
    knowledge = json.load(f)

# Extract key information
available_actions = knowledge['available_actions']
item_database = knowledge['common_items']
survival_tips = knowledge['survival_tips']
json_schemas = knowledge['json_schemas']
action_formats = knowledge['action_json_formats']

# Use knowledge to inform decisions
print("Available actions:", [act['action'] for act in available_actions])
print("Stimpak PID:", next(item['pid'] for item in item_database if item['name'] == 'Stimpak'))
```

### Knowledge Base Contents

The knowledge base includes everything an AI needs:

1. **Game Context** - Setting, objectives, gameplay style
2. **Action Dictionary** - Every available action with descriptions
3. **Item Database** - Important items with PIDs and usage tips
4. **Combat Mechanics** - How fighting works in detail
5. **SPECIAL & Skills** - What stats mean and how to use them
6. **Survival Guide** - 15+ tips for staying alive
7. **Control Instructions** - Step-by-step API usage
8. **JSON Formats** - Exact schemas for input/output
9. **Parsing Guide** - How to read/write JSON correctly
10. **Common Mistakes** - What to avoid and how to fix errors
11. **Decision Trees** - Logic for different situations
12. **Example Workflows** - Complete examples from start to finish

### Example: AI Agent with Knowledge Base

```python
import json
import time

class FalloutAI:
    def __init__(self):
        # Load knowledge once
        with open('ai_knowledge.json', 'r') as f:
            self.knowledge = json.load(f)
        
        # Build item lookup
        self.items = {item['name']: int(item['pid']) 
                     for item in self.knowledge['common_items']}
        
        # Learn action formats
        self.action_formats = {fmt['action_type']: fmt 
                              for fmt in self.knowledge['action_json_formats']}
    
    def decide_action(self, state):
        """Use knowledge and state to decide action"""
        # Check AI hints first
        if 'ai_hints' in state:
            for hint in state['ai_hints']:
                if hint['priority'] in ['CRITICAL', 'HIGH']:
                    # Parse suggested action from hint
                    if 'suggested_action' in hint:
                        return json.loads(hint['suggested_action'])
        
        # Use knowledge-based decision making
        hp_percent = state['hit_points'] / state['max_hit_points']
        
        if hp_percent < 0.3:
            # Knowledge says: heal when HP < 30%
            stimpak_pid = self.items.get('Stimpak', 40)
            return {'action': 'use_item', 'target_pid': stimpak_pid}
        
        if state['in_combat'] and state['nearby_objects']:
            # Find enemy and attack
            for obj in state['nearby_objects']:
                if obj['type'] == 1 and not obj.get('is_dead', True):
                    return {'action': 'attack', 'target_tile': obj['tile']}
        
        # Default: explore
        return {'action': 'move', 'target_tile': state['player_tile'] + 1}
    
    def play(self):
        """Main game loop with knowledge"""
        while True:
            # Read state
            with open('ai_state.json', 'r') as f:
                state = json.load(f)
            
            # Decide using knowledge
            action = self.decide_action(state)
            
            # Validate action format using knowledge
            action_type = action['action']
            if action_type in self.action_formats:
                format_info = self.action_formats[action_type]
                print(f"Using action: {action_type}")
                print(f"Required fields: {format_info['required_fields']}")
            
            # Execute action
            with open('ai_action.json', 'w') as f:
                json.dump(action, f)
            
            # Respect cooldown (from knowledge)
            time.sleep(0.1)  # 100ms cooldown
            
            # Check result
            print(f"Result: {state.get('last_action_result', 'none')}")

# Run AI with knowledge
ai = FalloutAI()
ai.play()
```

### Knowledge Base Advantages

- **No guessing** - AI knows exactly what actions exist and how to use them
- **Proper formatting** - JSON schema prevents syntax errors
- **Context aware** - Understands game mechanics and when to use actions
- **Self-documenting** - Complete reference for troubleshooting
- **Reduces hallucination** - AI has factual information about the game
- **Faster learning** - AI doesn't need to discover mechanics through trial/error

## Notes

- The API only works when a game is loaded (not in main menu)
- Actions are ignored if the game state doesn't support them
- State file is written using atomic operations to prevent corruption
- Action file is deleted after reading to prevent duplicate execution
- Event log wraps after 50 entries to maintain recent history
- Session stats reset when API is re-initialized
- **Knowledge base** is written once at startup and remains static
- **AI hints** in state provide dynamic, context-aware suggestions
- Use hints for immediate decisions, knowledge base for overall strategy

#### Attack Target
```json
{
  "action": "attack",
  "target_tile": 20105
}
```

## Implementation Details

- **Deterministic**: Actions are processed exactly once per frame
- **Low-overhead**: Minimal JSON parsing and writing
- **Safe**: Uses atomic file operations (write to temp, then rename)
- **Streaming-friendly**: Continuous state updates every frame

## Example Usage

Here's a simple Python example to interact with the API:

```python
import json
import time
import os

def read_state():
    if os.path.exists("ai_state.json"):
        with open("ai_state.json", "r") as f:
            return json.load(f)
    return None

def send_action(action, **kwargs):
    action_data = {"action": action}
    action_data.update(kwargs)
    with open("ai_action.json", "w") as f:
        json.dump(action_data, f)

# Main loop
while True:
    state = read_state()
    if state:
        print(f"Player at tile {state['player_tile']}, HP: {state['hit_points']}/{state['max_hit_points']}")
        
        # Example: Move to a nearby tile
        if not state["in_combat"]:
            send_action("move", target_tile=state["player_tile"] + 1)
    
    time.sleep(0.1)  # Check state 10 times per second
```

## Notes

- The API only works when a game is loaded (not in main menu)
- Actions are ignored if the game state doesn't support them (e.g., move during dialog)
- The state file is written using atomic operations to prevent corruption
- The action file is deleted after reading to prevent duplicate execution
