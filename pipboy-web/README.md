# Pip-Boy 2000 Web Interface

A web-based clone of the Fallout 1 Pip-Boy 2000, inspired by the Fallout 4 companion app. This allows you to control your in-game Pip-Boy from a web browser, managing inventory, stats, and character data without pausing gameplay.

## Features

### Core Pip-Boy Functionality
- **Status Tab**: View real-time HP, AP, radiation, and combat statistics
- **Stats Tab**: Browse SPECIAL attributes, skills, perks, and traits
- **Inventory Tab**: Manage items, weapons, and armor
- **Data Tab**: Access quests, notes, and world map
- **Real-time Sync**: Automatic updates from game state

### Character Profile System
- Browse Vault 13 personnel profiles
- Create and manage custom character profiles
- Import/export character data
- Profile templates for role-playing

## Quick Start

### Prerequisites
- Fallout 1 CE running with AI Control API enabled
- Node.js 18+ (for frontend)
- Python 3.8+ (for backend)

### Installation

1. **Start the Backend Server**
```bash
cd pipboy-web/backend
pip install -r requirements.txt
python server.py
```

The backend will run on `http://localhost:5001`

2. **Start the Frontend**
```bash
cd pipboy-web/frontend
npm install
npm run dev
```

The Pip-Boy interface will be available at `http://localhost:3001`

### Configuration

Edit `backend/config.json`:
```json
{
  "game_data_path": "../",
  "ai_state_file": "ai_state.json",
  "ai_action_file": "ai_action.json",
  "poll_interval": 0.5,
  "port": 5001
}
```

## Architecture

```
┌─────────────────┐
│  Fallout 1 Game │
│   (C++ Engine)  │
└────────┬────────┘
         │ writes JSON
         ▼
    ai_state.json ←──────┐
    ai_action.json       │ reads/writes
         │               │
         │          ┌────┴─────┐
         │          │  Backend │
         │          │  Python  │
         │          │  Flask   │
         │          └────┬─────┘
         │               │ REST API
         │               │ WebSocket
         │          ┌────┴─────┐
         │          │ Frontend │
         │          │ React/JS │
         └──────────┤ Pip-Boy  │
                    │    UI    │
                    └──────────┘
```

## Usage

### Accessing the Pip-Boy
1. Launch Fallout 1 CE with `ai_control_api=1` in fallout.cfg
2. Start the backend and frontend servers
3. Open your browser to `http://localhost:3001`
4. The Pip-Boy will automatically connect to the game

### Game Control
- **View Status**: Real-time updates of character stats
- **Manage Inventory**: Click items to use/equip (sends commands to game)
- **Browse Data**: View quests, locations, and notes
- **Character Profiles**: Access profile browser from main menu

### Character Profiles
Navigate to the Profile Browser to:
- View pre-defined Vault personnel profiles
- Create custom character builds
- Save and load character templates
- Share profiles (JSON export/import)

## Character Profile System

### Profile Structure
Profiles are stored in `pipboy-web/profiles/` as JSON files:

```json
{
  "id": "vault-dweller-001",
  "name": "Jack Morrison",
  "role": "Vault Dweller",
  "vault": "Vault 13",
  "background": "Selected to find the water chip...",
  "special": {
    "strength": 5,
    "perception": 7,
    "endurance": 6,
    "charisma": 4,
    "intelligence": 8,
    "agility": 7,
    "luck": 5
  },
  "tagSkills": ["Small Guns", "Science", "Speech"],
  "traits": ["Gifted", "Fast Shot"]
}
```

### Creating Custom Profiles
1. Copy `profiles/template.json` 
2. Edit the character details
3. Save with a unique filename
4. Refresh the profile browser

## Development

### Frontend Development
```bash
cd frontend
npm run dev      # Development server with hot reload
npm run build    # Production build
npm run preview  # Preview production build
```

### Backend Development
```bash
cd backend
python server.py --debug  # Run with debug logging
```

### File Structure
```
pipboy-web/
├── README.md                    # This file
├── backend/
│   ├── server.py               # Flask API server
│   ├── game_bridge.py          # Game communication layer
│   ├── profile_manager.py      # Profile CRUD operations
│   ├── requirements.txt        # Python dependencies
│   └── config.json             # Backend configuration
├── frontend/
│   ├── src/
│   │   ├── components/         # React components
│   │   │   ├── PipBoy.jsx     # Main Pip-Boy container
│   │   │   ├── StatusTab.jsx  # Status display
│   │   │   ├── StatsTab.jsx   # Stats/skills
│   │   │   ├── InventoryTab.jsx # Inventory management
│   │   │   ├── DataTab.jsx    # Quests and data
│   │   │   └── ProfileBrowser.jsx # Character profiles
│   │   ├── styles/            # CSS stylesheets
│   │   ├── utils/             # Helper functions
│   │   └── main.jsx           # Entry point
│   ├── public/                # Static assets
│   ├── package.json
│   └── vite.config.js
└── profiles/                   # Character profiles (JSON)
    ├── template.json
    ├── vault-dweller-001.json
    └── vault-personnel/        # Pre-defined profiles
```

## API Endpoints

### Game State
- `GET /api/status` - Current character status
- `GET /api/stats` - SPECIAL, skills, perks
- `GET /api/inventory` - Current inventory
- `GET /api/quests` - Quest log
- `GET /api/world-map` - Discovered locations

### Game Control
- `POST /api/inventory/use` - Use an item
- `POST /api/inventory/equip` - Equip weapon/armor
- `POST /api/inventory/drop` - Drop item
- `POST /api/rest` - Rest/heal

### Profiles
- `GET /api/profiles` - List all profiles
- `GET /api/profiles/:id` - Get specific profile
- `POST /api/profiles` - Create new profile
- `PUT /api/profiles/:id` - Update profile
- `DELETE /api/profiles/:id` - Delete profile

## Troubleshooting

### Connection Issues
- Ensure Fallout 1 is running with `ai_control_api=1`
- Check that `ai_state.json` is being created in game directory
- Verify backend is running and accessible
- Check browser console for errors

### Game Not Responding
- Commands are queued through `ai_action.json`
- Game must read and process the action file
- Some actions may take multiple frames
- Check backend logs for command status

### Profile Not Loading
- Verify JSON syntax is valid
- Check file permissions
- Ensure profile ID is unique
- Review backend logs for errors

## Security Notes

This application is designed for **personal local use only**:
- No authentication by default
- Runs on localhost
- Direct file system access
- Not intended for public deployment

For multi-user or remote access, implement:
- Authentication/authorization
- Input validation and sanitization
- Rate limiting
- Secure WebSocket connections

## Future Enhancements

- [ ] Real-time multiplayer profile sharing
- [ ] Voice commands for hands-free control
- [ ] Mobile-responsive design
- [ ] Character build optimizer
- [ ] Skill calculator and planning tools
- [ ] Integration with streaming overlays
- [ ] Save game management

## License

Same as Fallout 1 Community Edition - see LICENSE.md

## Credits

Inspired by:
- Fallout 4 Pip-Boy companion app
- Original Pip-Boy 2000 interface
- Fallout 1 Community Edition project
