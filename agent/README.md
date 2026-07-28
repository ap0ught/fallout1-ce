# Fallout 1 AI Agent Architecture

This AI agent reads game state from JSON files and makes intelligent decisions using three types of context:

## 1️⃣ Game State Context
Core game information the agent tracks:
- **HP**: Current and max health
- **Location**: Map name, tile position, elevation
- **Inventory**: Items, quantities, equipped gear
- **Quests**: Active objectives and progress
- **Time Elapsed**: Session time and turn count

## 2️⃣ Canon Context
Lore and world knowledge:
- **Where Am I?**: Location descriptions, significance, dangers
- **Who is this NPC?**: Character backgrounds, relationships, importance
- **What Do They Know?**: Quest information, secrets, trade goods

## 3️⃣ Strategy Context (Optional Toggle)
Gameplay optimization (can be disabled for harder/more immersive play):
- **Optimal Paths**: Best routes through areas, fastest quest solutions
- **Warnings**: Hidden dangers, trap locations, tough enemies ahead
- **Build Advice**: Skill recommendations, stat optimization, equipment suggestions

## Architecture

```
agent/
├── src/
│   ├── agent.py              # Main agent logic
│   ├── game_state.py         # Game state parser and tracker
│   ├── canon_context.py      # Lore and NPC knowledge database
│   ├── strategy_context.py   # Optimization and advice system
│   ├── decision_engine.py    # Makes decisions based on contexts
│   └── action_executor.py    # Writes actions to ai_action.json
├── data/
│   ├── locations.json        # Location descriptions and lore
│   ├── npcs.json            # NPC database with backgrounds
│   ├── quests.json          # Quest information and paths
│   └── strategies.json      # Optimal strategies and warnings
├── config.yaml              # Agent configuration
├── requirements.txt         # Python dependencies
└── README.md               # This file
```

## Usage

### Basic Usage
```bash
# Run agent with all contexts enabled
python agent/src/agent.py

# Run without strategy context (harder mode)
python agent/src/agent.py --no-strategy

# Run with custom LLM backend
python agent/src/agent.py --llm openai --model gpt-4
```

### Configuration

Edit `config.yaml` to customize:
```yaml
contexts:
  game_state: true      # Always enabled
  canon: true           # Lore and NPC knowledge
  strategy: false       # Optimization hints (toggle)

llm:
  provider: "openai"    # or "anthropic", "local"
  model: "gpt-4"
  temperature: 0.7

decision_making:
  risk_tolerance: "medium"  # low, medium, high
  exploration_bonus: 0.2
  combat_aggression: "defensive"  # defensive, balanced, aggressive
```

## Data Flow

```
Game (C++) 
  ↓ writes
ai_state.json ← Real-time game state
ai_memory.json ← Decision history
ai_knowledge.json ← Game mechanics
character_data.json ← Full character info
  ↓ reads
Agent (Python)
  ↓ processes with
[Game State Context] + [Canon Context] + [Strategy Context]
  ↓ decides action
Decision Engine
  ↓ writes
ai_action.json → Game reads and executes
```

## Context Details

### Game State Context
Parsed from `ai_state.json` and `character_data.json`:
- Current HP/AP status
- Location and nearby objects
- Inventory contents
- Active quest states
- Time and progress metrics

### Canon Context
Loaded from local databases and enriched with:
- Location lore from Fallout wiki
- NPC relationships and knowledge
- Quest storylines and outcomes
- World state and faction info

### Strategy Context (Optional)
Provides tactical advice:
- Optimal skill builds for playstyle
- Best paths through dangerous areas
- Warning about upcoming challenges
- Item and equipment recommendations
- Combat tactics for specific enemies

## Integration with LLMs

The agent can use various LLM backends:

1. **OpenAI GPT-4**: Best reasoning, expensive
2. **Anthropic Claude**: Great for roleplay
3. **Local Llama**: Free, private, slower
4. **Custom**: Bring your own model

The agent formats all three contexts into a prompt:
```
You are the Vault Dweller from Fallout 1.

GAME STATE:
HP: 45/60, Location: Junktown, AP: 8/10
Inventory: 10mm Pistol, 3 Stimpaks, Leather Armor

CANON CONTEXT:
Junktown is a small settlement built around an old gas station. 
Run by Killian Darkwater. Two factions: Killian (law) vs Gizmo (crime).

STRATEGY CONTEXT: [ENABLED]
Warning: Gizmo's casino has guards. Recommend talking to Killian first.
Optimal: Side with Killian for better long-term rewards.

What do you do?
```

## Features

- **Memory Integration**: Learns from `ai_memory.json` to avoid repeating mistakes
- **Character Consistency**: Stays in character using roleplay context
- **Adaptive Difficulty**: Strategy context can be toggled for challenge
- **Streaming Ready**: Works with Twitch chat integration
- **Explainable**: Logs reasoning for each decision
