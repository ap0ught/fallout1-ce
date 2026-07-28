// Mock data for development - matches extended JSON schema
export default {
  character: {
    name: 'Jack Morrison',
    age: 28,
    pronouns: 'he/him',
    origin: 'Vault 13',
    background: 'Vault Dweller',
    tagline: 'Chosen to find the water chip. The fate of Vault 13 rests on my shoulders.'
  },
  
  visuals: {
    portraitUrl: '/assets/portrait.png',
    spriteUrl: '/assets/sprite.gif',
    themeColor: '#0f0'
  },
  
  stats: {
    level: 5,
    experience: 4250,
    hp: 42,
    maxHp: 55,
    ap: 8,
    maxAp: 10,
    ac: 15,
    sequence: 12,
    healingRate: 2,
    criticalChance: 5
  },
  
  special: {
    strength: 6,
    perception: 7,
    endurance: 5,
    charisma: 4,
    intelligence: 8,
    agility: 7,
    luck: 5
  },
  
  skills: [
    { name: 'Small Guns', value: 65, tag: 'primary' },
    { name: 'Energy Weapons', value: 45, tag: 'primary' },
    { name: 'Science', value: 58, tag: 'primary' },
    { name: 'Lockpick', value: 52, tag: 'secondary' },
    { name: 'Speech', value: 48, tag: 'secondary' },
    { name: 'First Aid', value: 45, tag: 'secondary' },
    { name: 'Sneak', value: 42, tag: 'secondary' },
    { name: 'Repair', value: 38, tag: 'secondary' },
    { name: 'Big Guns', value: 25 },
    { name: 'Unarmed', value: 55 },
    { name: 'Melee Weapons', value: 40 },
    { name: 'Throwing', value: 35 },
    { name: 'Doctor', value: 30 },
    { name: 'Steal', value: 28 },
    { name: 'Traps', value: 25 },
    { name: 'Barter', value: 35 },
    { name: 'Gambling', value: 22 },
    { name: 'Outdoorsman', value: 40 }
  ],
  
  perks: [
    { name: 'Bonus Move', rank: 1, description: '+2 Action Points' },
    { name: 'More Criticals', rank: 1, description: '+5% Critical Chance' },
    { name: 'Educated', rank: 1, description: '+2 Skill Points per level' }
  ],
  
  traits: [
    { name: 'Gifted', description: '+1 to all SPECIAL, -10% to all Skills' },
    { name: 'Fast Shot', description: 'Faster attacks, cannot target specific body parts' }
  ],
  
  inventory: {
    equipped: [
      { slot: 'Weapon', name: '10mm Pistol', pid: 8 },
      { slot: 'Armor', name: 'Leather Armor', pid: 74 }
    ],
    notable: [
      { name: 'Stimpak', quantity: 8, pid: 40, note: 'Essential healing item' },
      { name: 'Rope', quantity: 2, pid: 127, note: 'Useful for climbing' },
      { name: 'Lockpicks', quantity: 4, pid: 84, note: 'For locked doors and containers' },
      { name: 'Water Flask', quantity: 3, pid: 101, note: 'Hydration for wasteland travel' },
      { name: 'Bottle Caps', quantity: 450, pid: 41, note: 'Currency' }
    ]
  },
  
  quests: [
    {
      id: 'waterchip',
      name: 'Find the Water Chip',
      status: 'active',
      highlight: true,
      description: 'The vault water chip is broken. Find a replacement within 150 days or the vault will perish.',
      linkedLocations: ['vault13', 'shady-sands', 'junktown']
    },
    {
      id: 'tandi-rescue',
      name: 'Rescue Tandi',
      status: 'completed',
      highlight: true,
      description: 'Rescued Aradesh\'s daughter from raiders.',
      outcome: 'Success - Gained reputation with Shady Sands',
      linkedLocations: ['shady-sands', 'raider-camp']
    },
    {
      id: 'stop-raiders',
      name: 'Stop the Raiders',
      status: 'completed',
      highlight: false,
      description: 'Eliminate the raider threat to Shady Sands.',
      outcome: 'Success - Raiders eliminated',
      linkedLocations: ['raider-camp']
    }
  ],
  
  journal: [
    {
      date: 'Day 45',
      entry: 'The wasteland is harsher than I ever imagined. Found Shady Sands - a small settlement trying to survive. They need help, but I can\'t forget my mission. 105 days left to find that water chip.',
      tags: ['shady-sands', 'wasteland']
    },
    {
      date: 'Day 48',
      entry: 'Rescued Tandi from raiders. Her father, Aradesh, is grateful. These people are good - they deserve better than constant fear. But I still haven\'t found any leads on the water chip.',
      tags: ['shady-sands', 'tandi', 'raiders']
    },
    {
      date: 'Day 52',
      entry: 'Decided to eliminate the raider camp. It was brutal, but necessary. The wasteland doesn\'t reward hesitation. Shady Sands should be safer now. Time to move on - heading to Junktown next.',
      tags: ['raiders', 'combat', 'junktown']
    }
  ],
  
  relations: {
    karma: 'Good (250)',
    factions: [
      { name: 'Shady Sands', reputation: 'Idolized', standing: 95 },
      { name: 'Junktown', reputation: 'Liked', standing: 60 },
      { name: 'Hub', reputation: 'Neutral', standing: 0 },
      { name: 'Brotherhood of Steel', reputation: 'Unknown', standing: 0 }
    ]
  },
  
  currentLocation: 'Junktown',
  
  map: {
    mapImage: '/assets/fallout1-map.png',
    locations: [
      {
        id: 'vault13',
        name: 'Vault 13',
        x: 30,
        y: 40,
        visited: true,
        type: 'vault'
      },
      {
        id: 'shady-sands',
        name: 'Shady Sands',
        x: 45,
        y: 45,
        visited: true,
        type: 'settlement'
      },
      {
        id: 'raider-camp',
        name: 'Raider Camp',
        x: 52,
        y: 38,
        visited: true,
        type: 'hostile'
      },
      {
        id: 'junktown',
        name: 'Junktown',
        x: 55,
        y: 55,
        visited: true,
        type: 'settlement'
      },
      {
        id: 'hub',
        name: 'The Hub',
        x: 62,
        y: 60,
        visited: false,
        type: 'city'
      },
      {
        id: 'necropolis',
        name: 'Necropolis',
        x: 48,
        y: 62,
        visited: false,
        type: 'ruins'
      }
    ],
    route: [
      { locationId: 'vault13', timestamp: 'Day 1', order: 1 },
      { locationId: 'shady-sands', timestamp: 'Day 45', order: 2 },
      { locationId: 'raider-camp', timestamp: 'Day 48', order: 3 },
      { locationId: 'junktown', timestamp: 'Day 52', order: 4 }
    ]
  },
  
  locations: {
    'vault13': {
      id: 'vault13',
      name: 'Vault 13',
      summary: 'Home. An underground sanctuary built by Vault-Tec to protect citizens from nuclear war. Population: 1000. Currently facing water chip failure.',
      firstArrival: 'Day 1 - Born here',
      visited: true,
      events: [
        { 
          id: 'e1',
          title: 'Chosen as the Vault Dweller',
          description: 'Selected by the Overseer to venture into the wasteland and find a replacement water chip.',
          linkedQuestId: 'waterchip',
          order: 1
        }
      ],
      npcs: [
        { name: 'Overseer', note: 'Leader of Vault 13. Gave me the mission.' },
        { name: 'Lyle', note: 'Weapons merchant. Gave me my first gun.' }
      ],
      tags: ['story-critical', 'vault', 'safe'],
      consequences: {
        karma: 0,
        reputation: {}
      }
    },
    'shady-sands': {
      id: 'shady-sands',
      name: 'Shady Sands',
      summary: 'A small farming community trying to survive in the harsh wasteland. Led by Aradesh. Under threat from raiders.',
      firstArrival: 'Day 45',
      visited: true,
      events: [
        {
          id: 'e2',
          title: 'First Contact with Wasteland Settlement',
          description: 'Met Aradesh and learned about the raiders threatening Shady Sands.',
          order: 2
        },
        {
          id: 'e3',
          title: 'Rescued Tandi',
          description: 'Saved Aradesh\'s daughter from the raider camp.',
          linkedQuestId: 'tandi-rescue',
          order: 3
        }
      ],
      npcs: [
        { name: 'Aradesh', note: 'Leader. Grateful for rescuing his daughter.' },
        { name: 'Tandi', note: 'Aradesh\'s daughter. Brave and resourceful.' },
        { name: 'Razlo', note: 'Town doctor. Taught me basic medical skills.' }
      ],
      tags: ['story-location', 'settlement', 'friendly', 'diplomacy-heavy'],
      consequences: {
        karma: +50,
        reputation: { 'Shady Sands': +40 }
      }
    },
    'raider-camp': {
      id: 'raider-camp',
      name: 'Raider Camp',
      summary: 'Hostile encampment of raiders preying on nearby settlements. Eliminated as a threat.',
      firstArrival: 'Day 48',
      visited: true,
      events: [
        {
          id: 'e4',
          title: 'Infiltrated Raider Camp',
          description: 'Snuck into the camp to rescue Tandi.',
          linkedQuestId: 'tandi-rescue',
          order: 4
        },
        {
          id: 'e5',
          title: 'Eliminated Raiders',
          description: 'Returned to wipe out the raider threat permanently.',
          linkedQuestId: 'stop-raiders',
          order: 5
        }
      ],
      npcs: [
        { name: 'Garl Death-Hand', note: 'Raider leader. Deceased.' }
      ],
      tags: ['combat-heavy', 'hostile', 'cleared'],
      consequences: {
        karma: +30,
        reputation: { 'Shady Sands': +55 }
      }
    },
    'junktown': {
      id: 'junktown',
      name: 'Junktown',
      summary: 'A trading post built from scrap. Gizmo runs the casino, while Killian Darkwater tries to keep order. Tensions are high.',
      firstArrival: 'Day 52',
      visited: true,
      events: [
        {
          id: 'e6',
          title: 'Arrived in Junktown',
          description: 'Found a larger settlement with more resources and opportunities.',
          order: 6
        }
      ],
      npcs: [
        { name: 'Killian Darkwater', note: 'Town leader. Runs the general store.' },
        { name: 'Gizmo', note: 'Casino owner. Seems shady.' },
        { name: 'Lars', note: 'Guard captain. Professional.' }
      ],
      tags: ['story-location', 'settlement', 'trading-hub', 'faction-conflict'],
      consequences: {
        karma: 0,
        reputation: {}
      }
    }
  },
  
  timeline: {
    entries: [
      {
        id: 't1',
        type: 'quest',
        date: 'Day 1',
        order: 1,
        title: 'Mission Received: Find the Water Chip',
        shortSummary: 'The Overseer chose me to save Vault 13. 150 days to find a water chip.',
        links: { questId: 'waterchip', locationId: 'vault13' }
      },
      {
        id: 't2',
        type: 'location',
        date: 'Day 45',
        order: 2,
        title: 'Discovered Shady Sands',
        shortSummary: 'First wasteland settlement. Small farming community under raider threat.',
        links: { locationId: 'shady-sands' }
      },
      {
        id: 't3',
        type: 'quest',
        date: 'Day 48',
        order: 3,
        title: 'Rescued Tandi from Raiders',
        shortSummary: 'Infiltrated raider camp and brought Aradesh\'s daughter home safely.',
        links: { questId: 'tandi-rescue', locationId: 'raider-camp' }
      },
      {
        id: 't4',
        type: 'combat',
        date: 'Day 50',
        order: 4,
        title: 'Eliminated Raider Threat',
        shortSummary: 'Wiped out the raider camp. Shady Sands is safer now.',
        links: { questId: 'stop-raiders', locationId: 'raider-camp' }
      },
      {
        id: 't5',
        type: 'location',
        date: 'Day 52',
        order: 5,
        title: 'Arrived at Junktown',
        shortSummary: 'Larger trading settlement. Potential leads on the water chip.',
        links: { locationId: 'junktown' }
      },
      {
        id: 't6',
        type: 'journal',
        date: 'Day 52',
        order: 6,
        title: 'Journal Entry: Reflection on Violence',
        shortSummary: 'The wasteland doesn\'t reward hesitation. Did what had to be done.',
        links: { journalId: 2 }
      }
    ]
  },
  
  streamHighlights: [
    'Currently in Junktown seeking water chip leads',
    'Rescued Tandi and eliminated raider threat at Shady Sands',
    'Level 5 Vault Dweller, specializing in Small Guns and Science',
    '105 days remaining to save Vault 13',
    'Idolized reputation with Shady Sands'
  ]
}
