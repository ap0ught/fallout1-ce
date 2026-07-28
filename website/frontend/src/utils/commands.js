// Mock data fetching (will be replaced with actual API calls)
import mockData from '../data/mockData'

export const COMMANDS = {
  HELP: {
    description: 'Display all available commands',
    execute: () => ({
      output: [
        '',
        '═══════════════════════════════════════════════════════════════',
        '  VAULT-TEC UNIFIED OPERATING SYSTEM - COMMAND REFERENCE',
        '═══════════════════════════════════════════════════════════════',
        '',
        '  HELP             - Display this command reference',
        '  CLEAR            - Clear terminal output',
        '  DOSSIER          - Display character summary overview',
        '  ID               - View Vault-Tec identification card',
        '  SHEET            - Display detailed character sheet',
        '  MAP              - Show interactive world map',
        '  LOCATIONS [id]   - Browse location archive or view specific location',
        '  TIMELINE [mode]  - View chronological event timeline',
        '                     Modes: QUICK, FULL, ARC (default: FULL)',
        '  QUESTS           - Display quest log',
        '  INVENTORY        - Show equipped items and notable possessions',
        '  JOURNAL          - Read in-character journal entries',
        '  RELATIONS        - View faction standings and reputation',
        '  SETTINGS         - Adjust terminal and display options',
        '',
        '  Navigation: Use arrow keys to recall previous commands',
        '  Tip: Commands are not case-sensitive',
        '',
        '═══════════════════════════════════════════════════════════════',
        ''
      ]
    })
  },

  CLEAR: {
    description: 'Clear terminal output',
    execute: () => ({
      output: [],
      clearPanel: true
    })
  },

  DOSSIER: {
    description: 'Display character summary overview',
    execute: () => ({
      output: [
        '',
        '╔══════════════════════════════════════════════════════════════╗',
        '║                    CHARACTER DOSSIER                         ║',
        '╚══════════════════════════════════════════════════════════════╝',
        '',
        `  NAME: ${mockData.character.name}`,
        `  ORIGIN: ${mockData.character.origin}`,
        `  LEVEL: ${mockData.stats.level}  |  AGE: ${mockData.character.age}  |  PRONOUNS: ${mockData.character.pronouns}`,
        '',
        `  ${mockData.character.tagline}`,
        '',
        '  ─── CURRENT STATUS ───',
        `  HP: ${mockData.stats.hp}/${mockData.stats.maxHp}  |  XP: ${mockData.stats.experience}`,
        `  LOCATION: ${mockData.currentLocation}`,
        `  KARMA: ${mockData.relations.karma}`,
        '',
        '  Use ID for detailed identification card.',
        '  Use SHEET for complete statistics.',
        ''
      ],
      panel: {
        type: 'dossier',
        data: mockData
      }
    })
  },

  ID: {
    description: 'View Vault-Tec identification card',
    execute: () => ({
      output: [
        '',
        '╔══════════════════════════════════════════════════════════════╗',
        '║              VAULT-TEC IDENTIFICATION CARD                   ║',
        '╚══════════════════════════════════════════════════════════════╝',
        '',
        '  Loading visual ID card...',
        '  See visual panel for complete identification document.',
        ''
      ],
      panel: {
        type: 'id',
        data: mockData
      }
    })
  },

  SHEET: {
    description: 'Display detailed character sheet',
    execute: () => ({
      output: [
        '',
        '╔══════════════════════════════════════════════════════════════╗',
        '║                   CHARACTER SHEET                            ║',
        '╚══════════════════════════════════════════════════════════════╝',
        '',
        '  ─── S.P.E.C.I.A.L. ───',
        `  Strength:     ${mockData.special.strength}`,
        `  Perception:   ${mockData.special.perception}`,
        `  Endurance:    ${mockData.special.endurance}`,
        `  Charisma:     ${mockData.special.charisma}`,
        `  Intelligence: ${mockData.special.intelligence}`,
        `  Agility:      ${mockData.special.agility}`,
        `  Luck:         ${mockData.special.luck}`,
        '',
        '  ─── TOP SKILLS ───',
        ...mockData.skills.slice(0, 5).map(s => `  ${s.name}: ${s.value}%`),
        '',
        '  See visual panel for complete details.',
        ''
      ],
      panel: {
        type: 'sheet',
        data: mockData
      }
    })
  },

  MAP: {
    description: 'Show interactive world map',
    execute: () => ({
      output: [
        '',
        '╔══════════════════════════════════════════════════════════════╗',
        '║                    WORLD MAP                                 ║',
        '╚══════════════════════════════════════════════════════════════╝',
        '',
        '  Loading interactive map...',
        '  Click locations in visual panel for details.',
        '',
        `  Locations Visited: ${mockData.map.locations.filter(l => l.visited).length}/${mockData.map.locations.length}`,
        ''
      ],
      panel: {
        type: 'map',
        data: mockData
      }
    })
  },

  LOCATIONS: {
    description: 'Browse location archive',
    execute: (args) => {
      const locationId = args[0]
      if (locationId) {
        const location = mockData.locations[locationId.toLowerCase()]
        if (location) {
          return {
            output: [
              '',
              `╔══════════════════════════════════════════════════════════════╗`,
              `║  LOCATION: ${location.name.toUpperCase().padEnd(51)}║`,
              `╚══════════════════════════════════════════════════════════════╝`,
              '',
              `  ${location.summary}`,
              '',
              `  First Arrival: ${location.firstArrival}`,
              `  Key Events: ${location.events.length}`,
              `  Notable NPCs: ${location.npcs.length}`,
              '',
              '  See visual panel for full details.',
              ''
            ],
            panel: {
              type: 'locations',
              data: { ...mockData, selectedLocation: locationId }
            }
          }
        } else {
          return {
            output: [`  ERROR: Location "${locationId}" not found.`]
          }
        }
      }
      return {
        output: [
          '',
          '╔══════════════════════════════════════════════════════════════╗',
          '║                  LOCATION ARCHIVE                            ║',
          '╚══════════════════════════════════════════════════════════════╝',
          '',
          `  Total Locations: ${Object.keys(mockData.locations).length}`,
          `  Visited: ${Object.values(mockData.locations).filter(l => l.visited).length}`,
          '',
          '  Use LOCATIONS [id] to view specific location.',
          '  See visual panel for filterable index.',
          ''
        ],
        panel: {
          type: 'locations',
          data: mockData
        }
      }
    }
  },

  TIMELINE: {
    description: 'View chronological event timeline',
    execute: (args) => {
      const mode = args[0] || 'FULL'
      let entries = mockData.timeline.entries
      
      if (mode === 'QUICK') {
        entries = entries.slice(0, 10)
      } else if (mode === 'ARC') {
        entries = entries.filter(e => e.type === 'quest' || e.type === 'milestone')
      }

      return {
        output: [
          '',
          '╔══════════════════════════════════════════════════════════════╗',
          `║  TIMELINE - ${mode} MODE                                     ║`,
          '╚══════════════════════════════════════════════════════════════╝',
          '',
          `  Showing ${entries.length} events`,
          '',
          '  Recent Events:',
          ...entries.slice(0, 5).map(e => `  ${e.order}. ${e.title}`),
          '',
          '  See visual panel for complete timeline.',
          ''
        ],
        panel: {
          type: 'timeline',
          data: { ...mockData, timelineMode: mode }
        }
      }
    }
  },

  QUESTS: {
    description: 'Display quest log',
    execute: () => ({
      output: [
        '',
        '╔══════════════════════════════════════════════════════════════╗',
        '║                     QUEST LOG                                ║',
        '╚══════════════════════════════════════════════════════════════╝',
        '',
        `  Active Quests: ${mockData.quests.filter(q => q.status === 'active').length}`,
        `  Completed: ${mockData.quests.filter(q => q.status === 'completed').length}`,
        `  Failed: ${mockData.quests.filter(q => q.status === 'failed').length}`,
        '',
        '  Highlights:',
        ...mockData.quests.filter(q => q.highlight).map(q => `  - ${q.name} [${q.status.toUpperCase()}]`),
        '',
        '  See visual panel for full quest details.',
        ''
      ],
      panel: {
        type: 'quests',
        data: mockData
      }
    })
  },

  INVENTORY: {
    description: 'Show equipped items and notable possessions',
    execute: () => ({
      output: [
        '',
        '╔══════════════════════════════════════════════════════════════╗',
        '║                     INVENTORY                                ║',
        '╚══════════════════════════════════════════════════════════════╝',
        '',
        '  ─── EQUIPPED ───',
        ...mockData.inventory.equipped.map(i => `  ${i.slot}: ${i.name}`),
        '',
        '  ─── NOTABLE ITEMS ───',
        ...mockData.inventory.notable.slice(0, 5).map(i => `  ${i.name} x${i.quantity}`),
        '',
        '  See visual panel for complete inventory.',
        ''
      ],
      panel: {
        type: 'inventory',
        data: mockData
      }
    })
  },

  JOURNAL: {
    description: 'Read in-character journal entries',
    execute: () => ({
      output: [
        '',
        '╔══════════════════════════════════════════════════════════════╗',
        '║                  PERSONAL JOURNAL                            ║',
        '╚══════════════════════════════════════════════════════════════╝',
        '',
        `  Total Entries: ${mockData.journal.length}`,
        '',
        '  Recent Entry:',
        `  "${mockData.journal[0]?.entry.substring(0, 60)}..."`,
        '',
        '  See visual panel for all entries.',
        ''
      ],
      panel: {
        type: 'journal',
        data: mockData
      }
    })
  },

  RELATIONS: {
    description: 'View faction standings and reputation',
    execute: () => ({
      output: [
        '',
        '╔══════════════════════════════════════════════════════════════╗',
        '║               FACTION RELATIONS                              ║',
        '╚══════════════════════════════════════════════════════════════╝',
        '',
        `  Overall Karma: ${mockData.relations.karma}`,
        '',
        '  ─── FACTION STANDINGS ───',
        ...mockData.relations.factions.map(f => `  ${f.name}: ${f.reputation}`),
        '',
        '  See visual panel for detailed reputation information.',
        ''
      ],
      panel: {
        type: 'relations',
        data: mockData
      }
    })
  },

  SETTINGS: {
    description: 'Adjust terminal and display options',
    execute: (args, { settings, setSettings }) => {
      if (args.length === 0) {
        return {
          output: [
            '',
            '╔══════════════════════════════════════════════════════════════╗',
            '║                      SETTINGS                                ║',
            '╚══════════════════════════════════════════════════════════════╝',
            '',
            '  Current Settings:',
            `  - CRT Intensity: ${settings.crtIntensity}`,
            `  - Scanlines: ${settings.scanlines ? 'ON' : 'OFF'}`,
            `  - Text Speed: ${settings.textSpeed}ms`,
            `  - Sound: ${settings.soundEnabled ? 'ON' : 'OFF'}`,
            `  - Stream Mode: ${settings.streamMode ? 'ON' : 'OFF'}`,
            '',
            '  Commands:',
            '  SETTINGS SCANLINES [ON|OFF]',
            '  SETTINGS SOUND [ON|OFF]',
            '  SETTINGS STREAM [ON|OFF]',
            ''
          ]
        }
      }

      const setting = args[0].toUpperCase()
      const value = args[1]?.toUpperCase()

      if (setting === 'SCANLINES') {
        return {
          output: [`  Scanlines: ${value === 'ON' ? 'ENABLED' : 'DISABLED'}`],
          settings: { scanlines: value === 'ON' }
        }
      } else if (setting === 'SOUND') {
        return {
          output: [`  Sound effects: ${value === 'ON' ? 'ENABLED' : 'DISABLED'}`],
          settings: { soundEnabled: value === 'ON' }
        }
      } else if (setting === 'STREAM') {
        return {
          output: [`  Stream Mode: ${value === 'ON' ? 'ENABLED - Enhanced readability for streaming' : 'DISABLED'}`],
          settings: { streamMode: value === 'ON' }
        }
      }

      return {
        output: [`  ERROR: Unknown setting "${setting}"`]
      }
    }
  }
}
