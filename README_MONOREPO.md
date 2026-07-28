# Fallout 1 AI Streaming Monorepo

Complete monorepo for AI-controlled Fallout 1 streaming with real-time web dashboard.

## Repository Structure

```
fallout1-ce/
├── src/                    # Game source code (C++)
├── agent/                  # AI agent that plays the game
│   ├── src/               # Agent Python code
│   ├── data/              # Lore databases (locations, NPCs, quests)
│   ├── config.yaml        # Agent configuration
│   └── requirements.txt   # Python dependencies
├── website/               # Web dashboard (for viewers)
│   ├── backend/           # Data collector & API server
│   ├── frontend/          # React dashboard UI
│   └── database/          # SQLite database for offline storage
├── pipboy-web/            # Pip-Boy 2000 web interface (personal control)
│   ├── backend/           # Flask API server & game bridge
│   ├── frontend/          # React Pip-Boy UI
│   ├── profiles/          # Character profiles (JSON)
│   ├── README.md          # Detailed documentation
│   └── QUICKSTART.md      # Quick setup guide
└── README_MONOREPO.md    # This file
```

## Quick Start

### 1. Build and Run the Game
```bash
# Build Fallout 1 with AI Control API
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .

# Enable AI API in config
echo "ai_control_api=1" >> fallout.cfg

# Run game
./fallout
```

### 2. Start the Website (Data Collector & Dashboard)
```bash
cd website/backend
pip install -r requirements.txt
python data_collector.py &  # Starts background collector
python api_server.py        # Starts web API

cd ../frontend
npm install
npm start                   # Dashboard at http://localhost:3000
```

### 3. Start the AI Agent
```bash
cd agent
pip install -r requirements.txt
python src/agent.py
```

## Data Flow

```
┌─────────────────┐
│  Fallout 1 Game │ (C++)
│   AI Control API│
└────────┬────────┘
         │ writes JSON files every frame
         │
         ├─ ai_state.json          (live state)
         ├─ ai_memory.json         (decision history)
         ├─ ai_knowledge.json      (game encyclopedia)
         └─ character_data.json    (full character info)
         │
         ├──────────────┬─────────────────┐
         │              │                 │
         ▼              ▼                 ▼
    ┌─────────┐   ┌──────────┐    ┌──────────┐
    │AI Agent │   │ Website  │    │ Twitch   │
    │         │   │ Collector│    │ Stream   │
    └────┬────┘   └────┬─────┘    └──────────┘
         │             │
         │ writes      │ reads & stores
         │             │
         ▼             ▼
    ai_action.json   SQLite DB
         │             │
         │             │ serves
         │             ▼
    ┌────┴──────┐  ┌──────────┐
    │   Game    │  │ Web      │
    │  Executes │  │Dashboard │
    └───────────┘  └──────────┘
```

## Components

### Game (Fallout 1 CE with AI Control API)
- **Location**: `src/game/ai_control_api.cc`
- **Outputs**: JSON files with game state
- **Language**: C++
- **Runs**: As main game process

**Exports every frame**:
- Current HP, AP, location, combat status
- Inventory, skills, stats
- Nearby objects, NPCs, enemies
- AI hints and suggestions

### AI Agent
- **Location**: `agent/`
- **Purpose**: Makes decisions and controls the game
- **Language**: Python
- **Runs**: As separate process

**Three Context Types**:
1. **Game State**: HP, Location, Inventory, Quests, Time
2. **Canon Context**: Where am I, Who is this NPC, What do they know
3. **Strategy Context** (optional): Optimal paths, Warnings, Build advice

### Website (Viewer Dashboard)
- **Location**: `website/`
- **Purpose**: Collects data and displays dashboard for viewers/audience
- **Languages**: Python (backend), React (frontend)
- **Runs**: As web server on port 3000/5000

**Components**:
- **Data Collector**: Reads JSON files, stores in SQLite
- **API Server**: Serves data to frontend
- **Frontend Dashboard**: Real-time character visualization

### Pip-Boy Web (Personal Control Interface)
- **Location**: `pipboy-web/`
- **Purpose**: Personal web-based Pip-Boy for game control (like Fallout 4 companion app)
- **Languages**: Python (backend), React (frontend)
- **Runs**: As web server on port 3001/5001

**Features**:
- **Real-time Game Control**: Manage inventory, use items, equip weapons
- **Status Monitoring**: View HP, AP, stats, skills in Pip-Boy style interface
- **Character Profiles**: Browse Vault personnel and create custom character builds
- **Quest & Map Tracking**: View active quests and discovered locations
- **Separate from Viewer Website**: This is YOUR personal control interface

See `pipboy-web/QUICKSTART.md` for setup instructions.

## Website Features

### Data Collection
- Polls JSON files every second
- Stores in SQLite for offline access
- Maintains complete history
- No data loss if game crashes

### Dashboard Displays
1. **Game State Panel**
   - HP bar with percentage
   - Current location
   - Inventory grid
   - Quest tracker
   - Time elapsed

2. **Canon Context Panel**
   - "Where Am I?" - Location lore
   - "Who is this NPC?" - Character backgrounds
   - "What do they know?" - Quest information

3. **Strategy Context Panel** (toggleable)
   - Optimal paths highlighted
   - Warnings about dangers
   - Build advice for character

4. **Journey Timeline**
   - Milestones achieved
   - Items collected history
   - Decisions made with outcomes

5. **Statistics**
   - Kills, damage dealt
   - Session time
   - Level progression graph

## Configuration

### Agent Configuration
Edit `agent/config.yaml`:
```yaml
contexts:
  canon: true          # Lore and NPC knowledge
  strategy: false      # Optimization (disable for hard mode)

llm:
  provider: "openai"
  model: "gpt-4"
```

### Website Configuration
Edit `website/backend/config.json`:
```json
{
  "game_data_path": "../../",
  "poll_interval": 1.0,
  "database_path": "./database/game_data.db",
  "api_port": 5000
}
```

## Development

### Adding New Agent Features
1. Add context in `agent/src/game_state.py`, `canon_context.py`, or `strategy_context.py`
2. Update decision logic in `agent/src/decision_engine.py`
3. Test with `python agent/src/agent.py --test`

### Adding New Website Features
1. Backend: Add API endpoint in `website/backend/api_server.py`
2. Frontend: Add React component in `website/frontend/src/components/`
3. Database: Update schema in `website/backend/database_schema.sql`

### Extending Game Data
1. Add new data exports in `src/game/ai_control_api.cc`
2. Update JSON schemas in documentation
3. Agent and website will automatically pick up new fields

## Streaming to Twitch

### Setup OBS
1. Add Browser Source
2. URL: `http://localhost:3000/overlay`
3. Width: 1920, Height: 1080
4. Custom CSS for transparency

### Enable Twitch Chat Voting
```yaml
# agent/config.yaml
twitch:
  enabled: true
  channel: "your_channel"
  chat_voting: true
```

### Recommended Layout
```
┌─────────────────────────────────────┐
│         Game Window (main)          │
│                                     │
│  ┌─────────────┐  ┌──────────────┐ │
│  │ HP/Stats    │  │ Recent Events│ │
│  │ (overlay)   │  │ (overlay)    │ │
│  └─────────────┘  └──────────────┘ │
└─────────────────────────────────────┘
```

## Database Schema

### Tables Created by Website
- `game_states` - Every state snapshot
- `inventory_snapshots` - Inventory over time
- `events` - All game events
- `milestones` - Major achievements
- `decisions` - AI decisions with reasoning
- `session_stats` - Aggregated statistics

### Queries Available
```sql
-- Get character progression
SELECT level, experience, timestamp FROM game_states ORDER BY timestamp;

-- Get all items collected
SELECT item_name, quantity, location, timestamp FROM inventory_snapshots;

-- Get decision history
SELECT action, reasoning, result FROM decisions;
```

## Troubleshooting

### Game not exporting data
- Check `ai_control_api=1` in fallout.cfg
- Verify files are being created in game directory
- Check file permissions

### Agent not making decisions
- Verify LLM API key is set
- Check `agent/logs/agent.log` for errors
- Test with `--test` flag

### Website not updating
- Check data collector is running
- Verify database path is correct
- Check browser console for errors

## Performance

- **Game**: ~1% overhead when API enabled
- **Agent**: ~1 action per second (configurable)
- **Website**: Updates every 1 second
- **Database**: Grows ~1MB per hour of gameplay

## License

Same as Fallout 1 Community Edition - see LICENSE.md
