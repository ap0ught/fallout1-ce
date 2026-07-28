# Fallout 1 Stream Companion - Frontend

Terminal-first website for AI-controlled Fallout 1 streaming. Provides a CRT terminal interface with visual panels for character data, maps, quests, and timeline.

## Features

### Core Terminal Experience
- **Full-screen CRT terminal** with scanlines, glow, and curvature effects
- **Boot sequence animation** for authentic Vault-Tec computer feel
- **Keyboard-first navigation** with command system (HELP, DOSSIER, ID, SHEET, MAP, etc.)
- **Split-view layout** - Terminal output + contextual visual panels
- **Stream Mode** - Enhanced readability for Twitch streaming

### Visual Panels

1. **ID Card** - Vault-Tec identification with character portrait and sprite
2. **Dossier** - Character summary with quick stats and highlights
3. **Character Sheet** - Full SPECIAL, skills, perks, traits, and derived stats
4. **Map** - Interactive Fallout 1 world map with visited locations and travel routes
5. **Locations** - Filterable location archive with detailed dossiers
6. **Timeline** - Chronological "Previously On..." feed (QUICK/FULL/ARC modes)
7. **Quests** - Quest log with status, outcomes, and highlights
8. **Inventory** - Equipped items and notable possessions
9. **Journal** - In-character diary entries
10. **Relations** - Faction reputation and karma tracking

### Settings & Accessibility
- **CRT Intensity** - Adjust scanline and glow effects
- **Scanlines Toggle** - On/off for visual comfort
- **Stream Mode** - Larger fonts and enhanced readability
- **Sound Toggle** - Optional SFX (boot beeps, keypress sounds)
- **Focus Modes** - FOCUS TERMINAL / FOCUS VISUAL / BALANCED

## Installation

```bash
npm install
```

## Development

Start the development server:

```bash
npm run dev
```

The application will be available at `http://localhost:3000`

The backend API should be running at `http://localhost:5000` (configured in `vite.config.js`)

## Building for Production

```bash
npm run build
```

Built files will be in the `dist` directory.

Preview the production build:

```bash
npm run preview
```

## Available Commands

Type these in the terminal prompt:

- `HELP` - Display all available commands
- `CLEAR` - Clear terminal output
- `DOSSIER` - Character summary overview
- `ID` - View Vault-Tec ID card
- `SHEET` - Display character sheet
- `MAP` - Show interactive world map
- `LOCATIONS [id]` - Browse locations or view specific location
- `TIMELINE [mode]` - View timeline (QUICK/FULL/ARC)
- `QUESTS` - Display quest log
- `INVENTORY` - Show inventory
- `JOURNAL` - Read journal entries
- `RELATIONS` - View faction standings
- `SETTINGS [option] [value]` - Adjust settings

## OBS/Streaming Setup

1. Add **Browser Source** to your OBS scene
2. Set URL to `http://localhost:3000`
3. Set dimensions (e.g., 1920x1080)
4. Enable "Refresh browser when scene becomes active"
5. Use **SETTINGS STREAM ON** command for stream-optimized display

## Data Source

The frontend displays data from:
- Backend API at `/api/*` endpoints (configured proxy)
- Mock data in `src/data/mockData.js` for development

## Styling

The application uses a Fallout 1 CRT terminal aesthetic:
- **Colors**: Amber/green phosphor text (#0f0, #ff0)
- **Effects**: Scanlines, CRT curvature (via SVG clip-path), glow, vignette
- **Font**: Courier New monospace
- **Style**: Vault-Tec bureaucratic design elements

## Project Structure

```
src/
├── components/
│   ├── Terminal.jsx          # Main terminal component
│   ├── BootSequence.jsx      # Boot animation
│   ├── CommandPrompt.jsx     # Command input
│   ├── VisualPanel.jsx       # Panel router
│   └── panels/               # Individual visual panels
│       ├── IdCard.jsx
│       ├── Dossier.jsx
│       ├── CharacterSheet.jsx
│       ├── Map.jsx
│       ├── Locations.jsx
│       ├── Timeline.jsx
│       ├── Quests.jsx
│       ├── Inventory.jsx
│       ├── Journal.jsx
│       └── Relations.jsx
├── styles/                   # CSS files
│   ├── index.css
│   ├── App.css
│   ├── Terminal.css
│   ├── BootSequence.css
│   ├── CommandPrompt.css
│   ├── VisualPanel.css
│   └── panels/               # Panel-specific styles
├── utils/
│   └── commands.js           # Command definitions and handlers
├── data/
│   └── mockData.js           # Sample data matching JSON schema
├── App.jsx                   # Root component
└── main.jsx                  # Entry point
```

## Development Notes

### Mock Data
The `mockData.js` file provides sample data for all panels during development. This follows the extended JSON schema that the backend should export.

### Command System
Commands are defined in `src/utils/commands.js`. Each command has:
- `description` - Help text
- `execute(args, context)` - Function that returns:
  - `output` - Array of terminal output lines
  - `panel` - Optional visual panel config `{ type, data }`
  - `settings` - Optional settings updates
  - `clearPanel` - Boolean to close visual panel

### Adding New Commands
1. Add command to `src/utils/commands.js`
2. Create visual panel component if needed in `src/components/panels/`
3. Add CSS styling in `src/styles/panels/`
4. Update VisualPanel.jsx to route the new panel type

## Browser Compatibility

Tested on:
- Chrome 90+
- Firefox 88+
- Safari 14+
- Edge 90+

Requires modern browser with ES6+ support and CSS clip-path.

## License

Same as Fallout 1 Community Edition - see LICENSE.md
