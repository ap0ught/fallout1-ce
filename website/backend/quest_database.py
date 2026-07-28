"""
Quest Database for Fallout 1

Maps quest GVARs (from game exports) to quest names, descriptions, locations, and objectives
based on the Fallout wiki and game data.

Sources:
- https://fallout.fandom.com/wiki/Fallout_quests
- https://fallout.fandom.com/wiki/Fallout_locations
- Game's GVAR definitions and quest scripts
"""

from typing import Dict, List, Any, Optional

# Quest status interpretation helpers
def interpret_quest_status(gvar_id: str, value: int) -> Dict[str, Any]:
    """
    Interpret GVAR quest value into human-readable status and outcome
    
    Args:
        gvar_id: GVAR identifier (e.g., "GVAR_RESCUE_TANDI")
        value: GVAR numeric value
        
    Returns:
        Dict with status, outcome, and progress information
    """
    # Most quests follow pattern: 0=not started, 1=started, 2=completed, negative=failed
    # But some quests have custom values
    
    if gvar_id in QUEST_DATABASE:
        quest = QUEST_DATABASE[gvar_id]
        status_map = quest.get('status_values', {})
        
        # Check for custom status interpretation
        if value in status_map:
            return status_map[value]
        
        # Default interpretation
        if value == 0:
            return {"status": "not_started", "outcome": None, "progress": 0}
        elif value < 0:
            return {"status": "failed", "outcome": "Quest failed", "progress": 100}
        elif value >= 2:
            return {"status": "completed", "outcome": "Quest completed", "progress": 100}
        else:
            return {"status": "active", "outcome": None, "progress": 50}
    
    # Fallback for unknown quests
    if value == 0:
        return {"status": "not_started", "outcome": None, "progress": 0}
    elif value < 0:
        return {"status": "failed", "outcome": "Failed", "progress": 100}
    elif value >= 2:
        return {"status": "completed", "outcome": "Completed", "progress": 100}
    else:
        return {"status": "active", "outcome": None, "progress": 50}


# Complete Quest Database
# Maps GVAR IDs to quest metadata
QUEST_DATABASE: Dict[str, Dict[str, Any]] = {
    # ========================================
    # MAIN QUEST LINE
    # ========================================
    
    "GVAR_FIND_WATER_CHIP": {
        "id": "find_water_chip",
        "name": "Find the Water Chip",
        "category": "main_quest",
        "location": "Vault 13",
        "description": "The water chip in Vault 13 has malfunctioned. Find a replacement before the vault runs out of water.",
        "objectives": [
            "Search for water chip information",
            "Investigate Vault 15",
            "Explore other vaults",
            "Find water merchants or technology",
            "Return water chip to Vault 13"
        ],
        "linked_locations": ["Vault 13", "Vault 15", "Necropolis", "The Hub"],
        "rewards": "Extended water supply for Vault 13",
        "wiki_url": "https://fallout.fandom.com/wiki/Find_the_Water_Chip",
        "status_values": {
            0: {"status": "active", "outcome": None, "progress": 10},
            1: {"status": "active", "outcome": "Searching for water chip", "progress": 50},
            2: {"status": "completed", "outcome": "Water chip delivered to Vault 13", "progress": 100}
        }
    },
    
    "GVAR_DESTROY_VATS": {
        "id": "destroy_mutant_vats",
        "name": "Destroy the Mutant Vats",
        "category": "main_quest",
        "location": "Mariposa Military Base",
        "description": "Destroy the vats at the Military Base that are creating Super Mutants.",
        "objectives": [
            "Locate the Military Base",
            "Infiltrate the facility",
            "Find the FEV vats",
            "Destroy the vats or set off the nuclear device"
        ],
        "linked_locations": ["Mariposa Military Base"],
        "rewards": "Halt Super Mutant production",
        "wiki_url": "https://fallout.fandom.com/wiki/Destroy_the_Mutant_army",
        "status_values": {
            0: {"status": "not_started", "outcome": None, "progress": 0},
            1: {"status": "active", "outcome": "Investigating mutant threat", "progress": 50},
            2: {"status": "completed", "outcome": "Vats destroyed", "progress": 100}
        }
    },
    
    "GVAR_DESTROY_MASTER": {
        "id": "destroy_master",
        "name": "Destroy the Master",
        "category": "main_quest",
        "location": "Cathedral",
        "description": "Defeat the Master and end his plans to transform humanity with the Forced Evolutionary Virus.",
        "objectives": [
            "Locate the Master's Cathedral",
            "Infiltrate the Cathedral",
            "Confront the Master",
            "Convince the Master or destroy him"
        ],
        "linked_locations": ["Cathedral", "Boneyard"],
        "rewards": "End the Super Mutant threat permanently",
        "wiki_url": "https://fallout.fandom.com/wiki/Destroy_the_source_of_the_mutants",
        "status_values": {
            0: {"status": "not_started", "outcome": None, "progress": 0},
            1: {"status": "active", "outcome": "Hunting the Master", "progress": 50},
            2: {"status": "completed", "outcome": "Master defeated", "progress": 100}
        }
    },
    
    # ========================================
    # CRITICAL TIMERS
    # ========================================
    
    "GVAR_VAULT_WATER": {
        "id": "vault_water_timer",
        "name": "Vault 13 Water Supply",
        "category": "timer",
        "location": "Vault 13",
        "description": "Days remaining before Vault 13 runs out of water. Extended by delivering water or finding the water chip.",
        "objectives": ["Find water chip before timer expires"],
        "linked_locations": ["Vault 13"],
        "rewards": "Vault 13 survival",
        "wiki_url": "https://fallout.fandom.com/wiki/Vault_13",
        "interpret_as_days": True
    },
    
    "GVAR_DAYS_TO_VAULT13_DISCOVERY": {
        "id": "vault_discovery_timer",
        "name": "Days Until Vault 13 Discovered",
        "category": "timer",
        "location": "Vault 13",
        "description": "Days until the Master's army discovers Vault 13. Affected by killing mutants near Vault 13.",
        "objectives": ["Complete main quest before discovery"],
        "linked_locations": ["Vault 13"],
        "rewards": "Vault 13 safety",
        "wiki_url": "https://fallout.fandom.com/wiki/Vault_13",
        "interpret_as_days": True
    },
    
    # ========================================
    # MAJOR SIDE QUESTS - SHADY SANDS
    # ========================================
    
    "GVAR_RESCUE_TANDI": {
        "id": "rescue_tandi",
        "name": "Rescue Tandi from the Raiders",
        "category": "side_quest",
        "location": "Shady Sands",
        "description": "Tandi, daughter of Aradesh, has been kidnapped by raiders. Rescue her and return her safely to Shady Sands.",
        "objectives": [
            "Talk to Aradesh about Tandi",
            "Locate the raider camp",
            "Rescue Tandi (fight, sneak, or negotiate)",
            "Return Tandi to Shady Sands"
        ],
        "linked_locations": ["Shady Sands", "Raider Camp"],
        "rewards": "500 XP, Aradesh's gratitude, improved Shady Sands reputation",
        "wiki_url": "https://fallout.fandom.com/wiki/Rescue_Tandi_from_the_Raiders",
        "status_values": {
            0: {"status": "not_started", "outcome": None, "progress": 0},
            1: {"status": "active", "outcome": "Searching for Tandi", "progress": 50},
            2: {"status": "completed", "outcome": "Tandi rescued and returned to Shady Sands", "progress": 100},
            -1: {"status": "failed", "outcome": "Tandi died", "progress": 100}
        }
    },
    
    "GVAR_KILL_RADSCORPIONS": {
        "id": "stop_radscorpions",
        "name": "Stop the Radscorpions",
        "category": "side_quest",
        "location": "Shady Sands",
        "description": "Radscorpions are threatening Shady Sands' crops. Eliminate the threat at their cave.",
        "objectives": [
            "Talk to Aradesh about the radscorpion problem",
            "Find the radscorpion cave",
            "Clear out the radscorpions",
            "Return to Aradesh"
        ],
        "linked_locations": ["Shady Sands", "Radscorpion Cave"],
        "rewards": "500 XP, improved Shady Sands reputation",
        "wiki_url": "https://fallout.fandom.com/wiki/Stop_the_Radscorpions",
        "status_values": {
            0: {"status": "not_started", "outcome": None, "progress": 0},
            1: {"status": "active", "outcome": "Hunting radscorpions", "progress": 50},
            2: {"status": "completed", "outcome": "Radscorpion threat eliminated", "progress": 100}
        }
    },
    
    # ========================================
    # MAJOR SIDE QUESTS - JUNKTOWN
    # ========================================
    
    "GVAR_KILL_KILLIAN": {
        "id": "junktown_conflict",
        "name": "Junktown Power Struggle",
        "category": "side_quest",
        "location": "Junktown",
        "description": "Choose between Killian Darkwater (mayor) and Gizmo (casino owner) in Junktown's power struggle.",
        "objectives": [
            "Investigate Gizmo's plans",
            "Side with Killian or Gizmo",
            "Determine Junktown's future"
        ],
        "linked_locations": ["Junktown"],
        "rewards": "Variable based on choice - affects Junktown's ending",
        "wiki_url": "https://fallout.fandom.com/wiki/Bust_the_Skulz_gang",
        "status_values": {
            0: {"status": "not_started", "outcome": None, "progress": 0},
            1: {"status": "active", "outcome": "Investigating Junktown politics", "progress": 50},
            2: {"status": "completed", "outcome": "Sided with Killian - Gizmo defeated", "progress": 100},
            3: {"status": "completed", "outcome": "Sided with Gizmo - Killian killed", "progress": 100}
        }
    },
    
    "GVAR_BUST_SKULZ": {
        "id": "bust_skulz",
        "name": "Bust the Skulz Gang",
        "category": "side_quest",
        "location": "Junktown",
        "description": "Lars has been robbed by the Skulz gang. Help bring them to justice.",
        "objectives": [
            "Talk to Lars about the theft",
            "Infiltrate the Skulz gang",
            "Get evidence of their crimes",
            "Report to Killian or handle directly"
        ],
        "linked_locations": ["Junktown"],
        "rewards": "500 XP, improved Junktown reputation",
        "wiki_url": "https://fallout.fandom.com/wiki/Bust_the_Skulz_gang",
        "status_values": {
            0: {"status": "not_started", "outcome": None, "progress": 0},
            1: {"status": "active", "outcome": "Investigating Skulz gang", "progress": 50},
            2: {"status": "completed", "outcome": "Skulz gang busted", "progress": 100}
        }
    },
    
    # ========================================
    # MAJOR SIDE QUESTS - THE HUB
    # ========================================
    
    "GVAR_KILL_DECKER": {
        "id": "decker_conspiracy",
        "name": "Stop Decker's Conspiracy",
        "category": "side_quest",
        "location": "The Hub",
        "description": "Decker, a crime boss in the Hub, is planning to assassinate the merchant council. Stop him.",
        "objectives": [
            "Get hired by Decker",
            "Learn about his plans",
            "Decide to betray him or complete his missions",
            "Confront Decker or report to authorities"
        ],
        "linked_locations": ["The Hub"],
        "rewards": "1000 XP, improved Hub reputation (if stopped)",
        "wiki_url": "https://fallout.fandom.com/wiki/Stop_Decker",
        "status_values": {
            0: {"status": "not_started", "outcome": None, "progress": 0},
            1: {"status": "active", "outcome": "Working for/against Decker", "progress": 50},
            2: {"status": "completed", "outcome": "Decker's conspiracy stopped", "progress": 100}
        }
    },
    
    "GVAR_FIND_MISSING_CARAVANS": {
        "id": "missing_caravans",
        "name": "Find the Missing Caravans",
        "category": "side_quest",
        "location": "The Hub",
        "description": "Caravans have been disappearing. Investigate and find out what happened to them.",
        "objectives": [
            "Talk to the Far Go Traders",
            "Investigate caravan routes",
            "Discover the source of attacks",
            "Report findings or eliminate threat"
        ],
        "linked_locations": ["The Hub"],
        "rewards": "500 XP, improved Hub reputation",
        "wiki_url": "https://fallout.fandom.com/wiki/Find_the_missing_caravans",
        "status_values": {
            0: {"status": "not_started", "outcome": None, "progress": 0},
            1: {"status": "active", "outcome": "Investigating caravan disappearances", "progress": 50},
            2: {"status": "completed", "outcome": "Mystery solved", "progress": 100}
        }
    },
    
    "GVAR_HUB_THIEVES_GUILD": {
        "id": "hub_thieves",
        "name": "Join the Thieves' Guild",
        "category": "side_quest",
        "location": "The Hub",
        "description": "Gain membership in the Hub's underground Thieves' Guild and complete missions.",
        "objectives": [
            "Find the Thieves' Circle",
            "Pass initiation test",
            "Complete guild missions"
        ],
        "linked_locations": ["The Hub"],
        "rewards": "Guild membership, access to merchants and quests",
        "wiki_url": "https://fallout.fandom.com/wiki/Steal_the_necklace",
        "status_values": {
            0: {"status": "not_started", "outcome": None, "progress": 0},
            1: {"status": "active", "outcome": "Completing guild initiation", "progress": 50},
            2: {"status": "completed", "outcome": "Full guild member", "progress": 100}
        }
    },
    
    # ========================================
    # MAJOR SIDE QUESTS - BROTHERHOOD OF STEEL
    # ========================================
    
    "GVAR_BROTHERHOOD_INITIATE": {
        "id": "brotherhood_initiate",
        "name": "Join the Brotherhood of Steel",
        "category": "side_quest",
        "location": "Brotherhood of Steel",
        "description": "Complete the initiation quest to become a member of the Brotherhood of Steel.",
        "objectives": [
            "Find the Brotherhood bunker",
            "Talk to the High Elder",
            "Accept initiation quest to The Glow",
            "Retrieve the Ancient Brotherhood disk",
            "Return to the Brotherhood"
        ],
        "linked_locations": ["Brotherhood of Steel", "The Glow"],
        "rewards": "Brotherhood membership, access to technology and quests",
        "wiki_url": "https://fallout.fandom.com/wiki/Initiation",
        "status_values": {
            0: {"status": "not_started", "outcome": None, "progress": 0},
            1: {"status": "active", "outcome": "Undertaking initiation quest", "progress": 50},
            2: {"status": "completed", "outcome": "Initiate of the Brotherhood", "progress": 100},
            -1: {"status": "failed", "outcome": "Failed initiation", "progress": 100}
        }
    },
    
    # ========================================
    # MAJOR SIDE QUESTS - NECROPOLIS
    # ========================================
    
    "GVAR_FIX_NECROPOLIS": {
        "id": "fix_necropolis_water",
        "name": "Fix the Necropolis Water Pump",
        "category": "side_quest",
        "location": "Necropolis",
        "description": "The ghouls of Necropolis need their water pump repaired. Help them or take their water chip.",
        "objectives": [
            "Find Necropolis",
            "Talk to the ghouls about the water problem",
            "Repair the water pump or steal the water chip",
            "Deal with Set, the ghoul leader"
        ],
        "linked_locations": ["Necropolis"],
        "rewards": "Variable - water chip (if stolen), ghoul gratitude (if repaired)",
        "wiki_url": "https://fallout.fandom.com/wiki/Fix_the_Necropolis_water_pump",
        "status_values": {
            0: {"status": "not_started", "outcome": None, "progress": 0},
            1: {"status": "active", "outcome": "Investigating Necropolis water situation", "progress": 50},
            2: {"status": "completed", "outcome": "Water pump repaired", "progress": 100},
            3: {"status": "completed", "outcome": "Water chip stolen", "progress": 100}
        }
    },
    
    # ========================================
    # MAJOR SIDE QUESTS - BONEYARD
    # ========================================
    
    "GVAR_BLADES_GUNS": {
        "id": "help_blades",
        "name": "Help the Blades",
        "category": "side_quest",
        "location": "Boneyard (Adytum)",
        "description": "The Blades gang needs weapons to fight the Regulators who secretly control Adytum.",
        "objectives": [
            "Meet the Blades",
            "Discover the Regulators' true nature",
            "Provide weapons to the Blades",
            "Help liberate Adytum"
        ],
        "linked_locations": ["Boneyard", "Adytum"],
        "rewards": "1000 XP, Boneyard liberation",
        "wiki_url": "https://fallout.fandom.com/wiki/Bring_the_guns_to_the_Blades",
        "status_values": {
            0: {"status": "not_started", "outcome": None, "progress": 0},
            1: {"status": "active", "outcome": "Helping the Blades", "progress": 50},
            2: {"status": "completed", "outcome": "Blades armed and Adytum liberated", "progress": 100}
        }
    },
    
    "GVAR_FOLLOWERS_INVASION": {
        "id": "followers_help",
        "name": "Help the Followers of the Apocalypse",
        "category": "side_quest",
        "location": "Boneyard",
        "description": "The Followers need assistance defending their library and helping the community.",
        "objectives": [
            "Find the Followers' library",
            "Talk to Nicole about helping",
            "Complete tasks for the Followers"
        ],
        "linked_locations": ["Boneyard"],
        "rewards": "XP, improved reputation, access to medical supplies",
        "wiki_url": "https://fallout.fandom.com/wiki/Followers_of_the_Apocalypse",
        "status_values": {
            0: {"status": "not_started", "outcome": None, "progress": 0},
            1: {"status": "active", "outcome": "Assisting Followers", "progress": 50},
            2: {"status": "completed", "outcome": "Followers helped", "progress": 100}
        }
    },
    
    # ========================================
    # SPECIAL ITEMS & ACHIEVEMENTS
    # ========================================
    
    "GVAR_PLAYER_GOT_CAR": {
        "id": "get_car",
        "name": "Acquire the Chryslus Highwayman",
        "category": "achievement",
        "location": "Various",
        "description": "Find and acquire the Chryslus Highwayman vehicle for faster wasteland travel.",
        "objectives": [
            "Find someone who can fix a car",
            "Locate car parts",
            "Pay for repairs"
        ],
        "linked_locations": ["The Hub", "Junktown"],
        "rewards": "Faster travel, reduced random encounters",
        "wiki_url": "https://fallout.fandom.com/wiki/Fallout_vehicles",
        "status_values": {
            0: {"status": "not_started", "outcome": None, "progress": 0},
            1: {"status": "completed", "outcome": "Vehicle acquired", "progress": 100}
        }
    },
    
    "GVAR_KILL_DEATHCLAW": {
        "id": "kill_deathclaw",
        "name": "Kill the Deathclaw",
        "category": "side_quest",
        "location": "Deathclaw's Lair",
        "description": "A deadly deathclaw is terrorizing the region. Hunt it down.",
        "objectives": [
            "Find the deathclaw's lair",
            "Defeat the deathclaw"
        ],
        "linked_locations": ["Boneyard"],
        "rewards": "1500 XP, deathclaw parts",
        "wiki_url": "https://fallout.fandom.com/wiki/Deathclaw_(Fallout)",
        "status_values": {
            0: {"status": "not_started", "outcome": None, "progress": 0},
            1: {"status": "active", "outcome": "Hunting the deathclaw", "progress": 50},
            2: {"status": "completed", "outcome": "Deathclaw slain", "progress": 100}
        }
    },
    
    # ========================================
    # ADDITIONAL TRACKED QUESTS
    # ========================================
    
    "GVAR_ARROYO_BRIDGE_BUILT": {
        "id": "build_bridge",
        "name": "Build the Bridge to Arroyo",
        "category": "side_quest",
        "location": "Arroyo",
        "description": "Help Arroyo by building a bridge across the canyon.",
        "objectives": ["Gather resources", "Complete bridge construction"],
        "linked_locations": ["Arroyo"],
        "rewards": "Improved Arroyo access",
        "status_values": {
            0: {"status": "not_started", "outcome": None, "progress": 0},
            1: {"status": "completed", "outcome": "Bridge completed", "progress": 100}
        }
    },
    
    "GVAR_FREE_ADYTUM": {
        "id": "free_adytum",
        "name": "Free Adytum from the Regulators",
        "category": "side_quest",
        "location": "Adytum",
        "description": "Expose the Regulators' tyranny and free the citizens of Adytum.",
        "objectives": [
            "Investigate the Regulators",
            "Gather evidence",
            "Defeat or expose the Regulators"
        ],
        "linked_locations": ["Adytum", "Boneyard"],
        "rewards": "1000 XP, Adytum liberation",
        "status_values": {
            0: {"status": "not_started", "outcome": None, "progress": 0},
            1: {"status": "active", "outcome": "Investigating Regulators", "progress": 50},
            2: {"status": "completed", "outcome": "Adytum freed", "progress": 100}
        }
    }
}


# Quest categories for filtering
QUEST_CATEGORIES = {
    "main_quest": "Main Quest",
    "side_quest": "Side Quest",
    "timer": "Critical Timer",
    "achievement": "Achievement"
}


def get_quest_info(gvar_id: str, gvar_value: int) -> Optional[Dict[str, Any]]:
    """
    Get complete quest information including status interpretation
    
    Args:
        gvar_id: GVAR identifier (e.g., "GVAR_RESCUE_TANDI")
        gvar_value: Current GVAR value from game
        
    Returns:
        Complete quest info dict or None if quest not in database
    """
    if gvar_id not in QUEST_DATABASE:
        return None
    
    quest = QUEST_DATABASE[gvar_id].copy()
    status_info = interpret_quest_status(gvar_id, gvar_value)
    
    # Merge status information
    quest.update(status_info)
    quest['gvar_value'] = gvar_value
    
    # Special handling for timer quests
    if quest.get('interpret_as_days'):
        quest['days_remaining'] = gvar_value
        quest['status'] = 'timer'
        quest['progress'] = None
    
    return quest


def get_active_quests(quest_gvars: Dict[str, int]) -> List[Dict[str, Any]]:
    """
    Get list of all active quests based on GVAR values
    
    Args:
        quest_gvars: Dict of GVAR_ID -> value pairs
        
    Returns:
        List of active quest dicts
    """
    active = []
    
    for gvar_id, value in quest_gvars.items():
        quest = get_quest_info(gvar_id, value)
        if quest and quest['status'] in ['active', 'timer']:
            active.append(quest)
    
    return active


def get_completed_quests(quest_gvars: Dict[str, int]) -> List[Dict[str, Any]]:
    """
    Get list of all completed quests based on GVAR values
    
    Args:
        quest_gvars: Dict of GVAR_ID -> value pairs
        
    Returns:
        List of completed quest dicts
    """
    completed = []
    
    for gvar_id, value in quest_gvars.items():
        quest = get_quest_info(gvar_id, value)
        if quest and quest['status'] == 'completed':
            completed.append(quest)
    
    return completed


def get_failed_quests(quest_gvars: Dict[str, int]) -> List[Dict[str, Any]]:
    """
    Get list of all failed quests based on GVAR values
    
    Args:
        quest_gvars: Dict of GVAR_ID -> value pairs
        
    Returns:
        List of failed quest dicts
    """
    failed = []
    
    for gvar_id, value in quest_gvars.items():
        quest = get_quest_info(gvar_id, value)
        if quest and quest['status'] == 'failed':
            failed.append(quest)
    
    return failed


def get_all_quests(quest_gvars: Dict[str, int]) -> Dict[str, List[Dict[str, Any]]]:
    """
    Get all quests organized by status
    
    Args:
        quest_gvars: Dict of GVAR_ID -> value pairs from game export
        
    Returns:
        Dict with 'active', 'completed', 'failed' quest lists
    """
    all_quests = {
        "active": [],
        "completed": [],
        "failed": [],
        "timers": []
    }
    
    for gvar_id, value in quest_gvars.items():
        quest = get_quest_info(gvar_id, value)
        if quest:
            if quest['status'] == 'timer':
                all_quests['timers'].append(quest)
            elif quest['status'] == 'active':
                all_quests['active'].append(quest)
            elif quest['status'] == 'completed':
                all_quests['completed'].append(quest)
            elif quest['status'] == 'failed':
                all_quests['failed'].append(quest)
    
    return all_quests


def get_quest_highlights(quest_gvars: Dict[str, int]) -> List[Dict[str, Any]]:
    """
    Get highlighted/important quests for stream display
    
    Args:
        quest_gvars: Dict of GVAR_ID -> value pairs
        
    Returns:
        List of important quests (main quests + active important side quests)
    """
    highlights = []
    
    # Always highlight main quests and timers if active
    main_quest_ids = ["GVAR_FIND_WATER_CHIP", "GVAR_DESTROY_VATS", "GVAR_DESTROY_MASTER", 
                      "GVAR_VAULT_WATER", "GVAR_DAYS_TO_VAULT13_DISCOVERY"]
    
    for gvar_id, value in quest_gvars.items():
        quest = get_quest_info(gvar_id, value)
        if quest:
            # Include main quests and timers if not completed
            if gvar_id in main_quest_ids and quest['status'] in ['active', 'timer']:
                highlights.append(quest)
            # Include any active side quest
            elif quest['status'] == 'active' and quest.get('category') == 'side_quest':
                highlights.append(quest)
    
    # Limit to top 5 most important
    return highlights[:5]
