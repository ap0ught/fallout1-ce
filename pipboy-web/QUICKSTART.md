# Pip-Boy 2000 Quick Start Guide

## What This Does

This is a **personal web-based Pip-Boy interface** that lets you control Fallout 1 from your browser. It's completely separate from the viewer website - this is YOUR control interface.

Features:
- View real-time game stats (HP, AP, level, location)
- Manage inventory (use items, equip weapons/armor, drop items)
- Browse quests and world map
- Create and manage character profiles for role-playing

## Installation (5 minutes)

### Step 1: Install Backend Dependencies

```bash
cd pipboy-web/backend
pip install -r requirements.txt
```

### Step 2: Install Frontend Dependencies

```bash
cd pipboy-web/frontend
npm install
```

### Step 3: Configure Game

Make sure your Fallout 1 has AI Control API enabled:

```
# Add to fallout.cfg
ai_control_api=1
```

## Running the Pip-Boy

### Terminal 1 - Start Backend Server

```bash
cd pipboy-web/backend
python server.py
```

The backend runs on `http://localhost:5001`

### Terminal 2 - Start Frontend

```bash
cd pipboy-web/frontend
npm run dev
```

The Pip-Boy interface runs on `http://localhost:3001`

### Terminal 3 - Start the Game

```bash
# Navigate to your Fallout 1 directory and run the game
./fallout-ce
```

## Using the Pip-Boy

1. Open your browser to `http://localhost:3001`
2. You'll see the classic green Pip-Boy interface
3. Use the tabs:
   - **STAT**: View HP, AP, and current status
   - **SPECIAL**: View S.P.E.C.I.A.L. stats, skills, perks
   - **INV**: Manage your inventory (use/equip/drop items)
   - **DATA**: View quests and world map

4. Click the **PROFILES** button to:
   - Browse Vault 13 personnel profiles
   - Create custom character builds
   - Manage your personal characters

## Character Profiles

### Browsing Existing Profiles

- Pre-loaded profiles include Overseer, Lyle, and the default Vault Dweller
- Click any profile to view their stats, background, and relationships

### Creating Your Own

1. Click **PROFILES** in the main menu
2. Click **+ New Profile**
3. Fill in character details:
   - Name, age, gender, role
   - Background story and tagline
   - S.P.E.C.I.A.L. attributes
   - Tag skills and traits
4. Click **Save Profile**

### Profile Files

Profiles are stored as JSON in `pipboy-web/profiles/`:
- `vault-personnel/` - Pre-defined Vault characters (read-only suggested)
- Root directory - Your custom characters

## Tips

### Controlling the Game

- Commands are sent through the API
- Some actions may take a moment to process
- Watch the connection status indicator (top-right)

### Adding More Profiles

You can manually create profile JSON files:

```bash
cp pipboy-web/profiles/template.json pipboy-web/profiles/my-character.json
# Edit my-character.json with your character details
```

### Customization

Edit `pipboy-web/backend/config.json` to change:
- Server port
- Game data path
- Polling interval

### Troubleshooting

**Backend won't start:**
- Make sure Python 3.8+ is installed
- Check that port 5001 isn't in use

**Frontend won't start:**
- Make sure Node.js 18+ is installed
- Try `rm -rf node_modules && npm install`

**Not connecting to game:**
- Verify `ai_control_api=1` in fallout.cfg
- Check that Fallout 1 is running
- Look for `ai_state.json` in your game directory

**No game data showing:**
- The game must be actively running and exporting data
- Check that file paths in `config.json` are correct

## What's Different from the Viewer Website?

| Feature | Viewer Website | Pip-Boy Web |
|---------|---------------|-------------|
| **Purpose** | For audience to watch | For you to control game |
| **Port** | 3000 | 3001 |
| **Features** | View-only dashboard | Interactive game control |
| **Profiles** | No | Yes - create & manage |
| **Inventory** | View only | Use/equip/drop items |
| **Backend** | 5000 | 5001 |

## Next Steps

- Create character profiles for role-playing
- Use inventory management during gameplay
- Track quests without opening in-game Pip-Boy
- Experiment with the API for custom integrations

## Need Help?

Check the main README.md for detailed documentation or report issues on GitHub.

Enjoy your Pip-Boy 2000! 🎮
