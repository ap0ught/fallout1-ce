"""
Extended Character Data Generator

Generates comprehensive character data JSON with timeline, locations, quests, journal.
This module extends the basic game state with features needed for the stream companion UI.
"""

import json
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Any

# Import quest database for wiki-based quest information
from quest_database import (
    get_all_quests,
    get_quest_highlights,
    QUEST_CATEGORIES
)

class CharacterDataGenerator:
    """Generates extended character data for frontend"""
    
    def __init__(self, game_data_dir: str = "../.."):
        self.game_data_dir = Path(game_data_dir)
        self.timeline_entries = []
        self.locations_visited = {}
        self.journal_entries = []
        self.quest_log = {}
        self.location_events = {}
        self.faction_reputation = {}
        
    def load_base_game_data(self) -> Dict[str, Any]:
        """Load existing game data from JSON files"""
        base_data = {}
        
        # Load ai_state.json
        state_file = self.game_data_dir / "ai_state.json"
        if state_file.exists():
            with open(state_file, 'r') as f:
                base_data['state'] = json.load(f)
        else:
            base_data['state'] = self._get_default_state()
        
        # Load character_data.json if it exists
        char_file = self.game_data_dir / "character_data.json"
        if char_file.exists():
            with open(char_file, 'r') as f:
                base_data['character'] = json.load(f)
        else:
            base_data['character'] = {}
            
        # Load ai_memory.json
        memory_file = self.game_data_dir / "ai_memory.json"
        if memory_file.exists():
            with open(memory_file, 'r') as f:
                base_data['memory'] = json.load(f)
        else:
            base_data['memory'] = {'memories': []}
            
        return base_data
    
    def _get_default_state(self) -> Dict[str, Any]:
        """Default state when no game data available"""
        return {
            'hit_points': 50,
            'max_hit_points': 50,
            'action_points': 10,
            'max_action_points': 10,
            'level': 1,
            'experience': 0,
            'armor_class': 5,
            'map_name': 'Vault 13',
            'strength': 5,
            'perception': 5,
            'endurance': 5,
            'charisma': 5,
            'intelligence': 5,
            'agility': 5,
            'luck': 5,
            'skills': [],
            'perks': [],
            'inventory': []
        }
    
    def generate_extended_data(self) -> Dict[str, Any]:
        """Generate complete extended character data"""
        base_data = self.load_base_game_data()
        state = base_data.get('state', {})
        
        extended_data = {
            "character": self._generate_character_info(state),
            "visuals": self._generate_visual_data(),
            "stats": self._generate_stats(state),
            "special": self._extract_special(state),
            "skills": self._generate_skills(state),
            "perks": self._extract_perks(state),
            "traits": self._generate_traits(),
            "inventory": self._generate_inventory(state),
            "quests": self._generate_quests(base_data),
            "journal": self._generate_journal(base_data),
            "relations": self._generate_relations(),
            "currentLocation": state.get('map_name', 'Unknown'),
            "map": self._generate_map_data(base_data),
            "locations": self._generate_locations(base_data),
            "timeline": self._generate_timeline(base_data),
            "streamHighlights": self._generate_highlights(state)
        }
        
        return extended_data
    
    def _generate_character_info(self, state: Dict) -> Dict:
        """Generate character identity information"""
        return {
            "name": state.get('character_name', 'Vault Dweller'),
            "age": 28,  # Default, could be configured
            "pronouns": "they/them",  # Default, could be configured
            "origin": "Vault 13",
            "background": "Vault Dweller",
            "tagline": "Chosen to find the water chip and save Vault 13."
        }
    
    def _generate_visual_data(self) -> Dict:
        """Generate visual assets references"""
        return {
            "portraitUrl": "/assets/portrait.png",
            "spriteUrl": "/assets/sprite.gif",
            "themeColor": "#0f0"
        }
    
    def _generate_stats(self, state: Dict) -> Dict:
        """Generate derived stats"""
        return {
            "level": state.get('level', 1),
            "experience": state.get('experience', 0),
            "hp": state.get('hit_points', 50),
            "maxHp": state.get('max_hit_points', 50),
            "ap": state.get('action_points', 10),
            "maxAp": state.get('max_action_points', 10),
            "ac": state.get('armor_class', 5),
            "sequence": state.get('sequence', 10),
            "healingRate": 1,  # Could calculate from endurance
            "criticalChance": 5  # Could calculate from luck
        }
    
    def _extract_special(self, state: Dict) -> Dict:
        """Extract SPECIAL attributes"""
        return {
            "strength": state.get('strength', 5),
            "perception": state.get('perception', 5),
            "endurance": state.get('endurance', 5),
            "charisma": state.get('charisma', 5),
            "intelligence": state.get('intelligence', 5),
            "agility": state.get('agility', 5),
            "luck": state.get('luck', 5)
        }
    
    def _generate_skills(self, state: Dict) -> List[Dict]:
        """Generate skills list with tags"""
        skills = state.get('skills', [])
        
        # If we have skills, process them
        if skills:
            processed_skills = []
            for skill in skills:
                skill_entry = {
                    "name": skill.get('name', 'Unknown'),
                    "value": skill.get('value', 0)
                }
                
                # Tag high-value skills
                if skill_entry['value'] >= 50:
                    skill_entry['tag'] = 'primary'
                elif skill_entry['value'] >= 35:
                    skill_entry['tag'] = 'secondary'
                    
                processed_skills.append(skill_entry)
            
            # Sort by value
            processed_skills.sort(key=lambda x: x['value'], reverse=True)
            return processed_skills
        
        # Default skills if none provided
        return [
            {"name": "Small Guns", "value": 35, "tag": "primary"},
            {"name": "First Aid", "value": 30, "tag": "secondary"},
            {"name": "Speech", "value": 25}
        ]
    
    def _extract_perks(self, state: Dict) -> List[Dict]:
        """Extract perks with descriptions"""
        perks = state.get('perks', [])
        
        # Add descriptions if not present
        perk_descriptions = {
            "Bonus Move": "+2 Action Points",
            "More Criticals": "+5% Critical Chance",
            "Educated": "+2 Skill Points per level",
            "Toughness": "+10% Damage Resistance"
        }
        
        processed_perks = []
        for perk in perks:
            perk_name = perk.get('name', 'Unknown')
            processed_perks.append({
                "name": perk_name,
                "rank": perk.get('level', 1),
                "description": perk_descriptions.get(perk_name, "Perk bonus")
            })
        
        return processed_perks
    
    def _generate_traits(self) -> List[Dict]:
        """Generate character traits (usually set at creation)"""
        # These would typically be loaded from game data
        # For now, return empty list
        return []
    
    def _generate_inventory(self, state: Dict) -> Dict:
        """Generate inventory with equipped and notable items"""
        inventory = state.get('inventory', [])
        
        equipped = []
        notable = []
        
        # Process inventory items
        for item in inventory:
            pid = item.get('pid', 0)
            name = item.get('name', 'Unknown')
            quantity = item.get('quantity', 1)
            
            # Check if it's a weapon or armor (would need PID lookups)
            if pid in [8, 9, 10, 18]:  # Example weapon PIDs
                equipped.append({
                    "slot": "Weapon",
                    "name": name,
                    "pid": pid
                })
            elif pid in [74, 75, 113]:  # Example armor PIDs
                equipped.append({
                    "slot": "Armor",
                    "name": name,
                    "pid": pid
                })
            elif pid in [40, 144, 273]:  # Notable items (Stimpak, Rad-X, RadAway)
                notable.append({
                    "name": name,
                    "quantity": quantity,
                    "pid": pid,
                    "note": self._get_item_note(pid)
                })
        
        return {
            "equipped": equipped,
            "notable": notable
        }
    
    def _get_item_note(self, pid: int) -> str:
        """Get flavor note for an item"""
        notes = {
            40: "Essential healing item",
            144: "Radiation resistance",
            273: "Remove radiation",
            127: "Useful for climbing",
            84: "For locked doors and containers"
        }
        return notes.get(pid, "Useful item")
    
    def _generate_quests(self, base_data: Dict) -> List[Dict]:
        """Generate quest log from game state using quest database"""
        state = base_data.get('state', {})
        
        # Get quest GVARs from game export (v2.2.0+)
        quest_gvars = state.get('quests', {})
        
        # If no quest data from game, return minimal fallback
        if not quest_gvars:
            return [{
                "id": "waterchip",
                "name": "Find the Water Chip",
                "status": "active",
                "category": "main_quest",
                "highlight": True,
                "description": "The vault water chip is broken. Find a replacement.",
                "linkedLocations": ["vault13"],
                "source": "fallback"
            }]
        
        # Use quest database to get full quest information
        all_quests = get_all_quests(quest_gvars)
        
        # Flatten and format for frontend
        quest_list = []
        
        # Add timers first (always prominent)
        for quest in all_quests.get('timers', []):
            quest_list.append(self._format_quest_for_frontend(quest))
        
        # Add active quests
        for quest in all_quests.get('active', []):
            quest_list.append(self._format_quest_for_frontend(quest))
        
        # Add completed quests
        for quest in all_quests.get('completed', []):
            quest_list.append(self._format_quest_for_frontend(quest))
        
        # Add failed quests
        for quest in all_quests.get('failed', []):
            quest_list.append(self._format_quest_for_frontend(quest))
        
        return quest_list
    
    def _format_quest_for_frontend(self, quest: Dict) -> Dict:
        """Format quest data for frontend consumption"""
        formatted = {
            "id": quest.get('id'),
            "name": quest.get('name'),
            "status": quest.get('status'),
            "category": QUEST_CATEGORIES.get(quest.get('category', 'side_quest'), 'Side Quest'),
            "description": quest.get('description', ''),
            "linkedLocations": quest.get('linked_locations', []),
            "source": "wiki"
        }
        
        # Add progress if available
        if quest.get('progress') is not None:
            formatted['progress'] = quest['progress']
        
        # Add outcome for completed/failed quests
        if quest.get('outcome'):
            formatted['outcome'] = quest['outcome']
        
        # Add objectives if available
        if quest.get('objectives'):
            formatted['objectives'] = quest['objectives']
        
        # Add rewards if available
        if quest.get('rewards'):
            formatted['rewards'] = quest['rewards']
        
        # Add wiki URL for reference
        if quest.get('wiki_url'):
            formatted['wiki_url'] = quest['wiki_url']
        
        # Special handling for timer quests
        if quest.get('days_remaining') is not None:
            formatted['days_remaining'] = quest['days_remaining']
            formatted['highlight'] = True
        
        # Highlight main quests
        if quest.get('category') == 'main_quest':
            formatted['highlight'] = True
        else:
            formatted['highlight'] = False
        
        return formatted
    
    def _generate_journal(self, base_data: Dict) -> List[Dict]:
        """Generate journal entries from session"""
        journal = []
        
        # Could generate from memories, for now use template
        state = base_data.get('state', {})
        day = state.get('session_time_seconds', 0) // 86400 + 1
        
        journal.append({
            "date": f"Day {day}",
            "entry": f"The wasteland is harsh, but I must find the water chip. Currently at {state.get('map_name', 'Unknown')}. The vault is counting on me.",
            "tags": [state.get('map_name', 'wasteland')]
        })
        
        return journal
    
    def _generate_relations(self) -> Dict:
        """Generate faction relations"""
        return {
            "karma": "Neutral (0)",
            "factions": [
                {"name": "Vault 13", "reputation": "Idolized", "standing": 100},
            ]
        }
    
    def _generate_map_data(self, base_data: Dict) -> Dict:
        """Generate map with locations and routes"""
        # Track visited locations from memories
        memories = base_data.get('memory', {}).get('memories', [])
        visited_maps = set()
        route = []
        
        for idx, memory in enumerate(memories):
            map_name = memory.get('map', '')
            if map_name and map_name not in visited_maps:
                visited_maps.add(map_name)
                route.append({
                    "locationId": self._normalize_location_id(map_name),
                    "timestamp": f"Visit {len(route) + 1}",
                    "order": len(route) + 1
                })
        
        # Default locations
        locations = [
            {"id": "vault13", "name": "Vault 13", "x": 30, "y": 40, "visited": True, "type": "vault"},
            {"id": "shady-sands", "name": "Shady Sands", "x": 45, "y": 45, "visited": False, "type": "settlement"},
            {"id": "junktown", "name": "Junktown", "x": 55, "y": 55, "visited": False, "type": "settlement"},
        ]
        
        # Mark visited locations
        for loc in locations:
            if loc['id'] in visited_maps or loc['name'] in visited_maps:
                loc['visited'] = True
        
        return {
            "mapImage": "/assets/fallout1-map.png",
            "locations": locations,
            "route": route if route else [{"locationId": "vault13", "timestamp": "Day 1", "order": 1}]
        }
    
    def _normalize_location_id(self, map_name: str) -> str:
        """Convert map name to location ID"""
        return map_name.lower().replace(' ', '-').replace('_', '-')
    
    def _generate_locations(self, base_data: Dict) -> Dict:
        """Generate detailed location dossiers"""
        state = base_data.get('state', {})
        current_map = state.get('map_name', 'Unknown')
        
        locations = {
            "vault13": {
                "id": "vault13",
                "name": "Vault 13",
                "summary": "Home. Underground sanctuary built by Vault-Tec. Population: 1000. Currently facing water chip failure.",
                "firstArrival": "Day 1 - Born here",
                "visited": True,
                "events": [{
                    "id": "e1",
                    "title": "Chosen as the Vault Dweller",
                    "description": "Selected by the Overseer to find a replacement water chip."
                }],
                "npcs": [
                    {"name": "Overseer", "note": "Leader of Vault 13. Gave me the mission."}
                ],
                "tags": ["story-critical", "vault", "safe"],
                "consequences": {"karma": 0, "reputation": {}}
            }
        }
        
        # Add current location if not already present
        if current_map and self._normalize_location_id(current_map) not in locations:
            loc_id = self._normalize_location_id(current_map)
            locations[loc_id] = {
                "id": loc_id,
                "name": current_map,
                "summary": f"Currently exploring {current_map}.",
                "firstArrival": "Recent",
                "visited": True,
                "events": [],
                "npcs": [],
                "tags": ["explored"],
                "consequences": {"karma": 0, "reputation": {}}
            }
        
        return locations
    
    def _generate_timeline(self, base_data: Dict) -> Dict:
        """Generate timeline from memories"""
        entries = []
        
        # Starting entry
        entries.append({
            "id": "t1",
            "type": "quest",
            "date": "Day 1",
            "order": 1,
            "title": "Mission Received: Find the Water Chip",
            "shortSummary": "The Overseer chose me to save Vault 13. 150 days to find a water chip.",
            "links": {"questId": "waterchip", "locationId": "vault13"}
        })
        
        # Parse memories for timeline events
        memories = base_data.get('memory', {}).get('memories', [])
        for idx, memory in enumerate(memories[:20]):  # Limit to recent 20
            action = memory.get('action', '')
            result = memory.get('result', '')
            map_name = memory.get('map', 'Unknown')
            
            entry_type = 'combat' if action == 'attack' else 'location'
            
            entries.append({
                "id": f"t{idx + 2}",
                "type": entry_type,
                "date": f"Event {idx + 1}",
                "order": idx + 2,
                "title": f"{action.title()}: {result[:50]}",
                "shortSummary": result,
                "links": {"locationId": self._normalize_location_id(map_name)}
            })
        
        return {"entries": entries}
    
    def _generate_highlights(self, state: Dict) -> List[str]:
        """Generate stream highlights using quest database"""
        highlights = []
        
        # Character level and location
        highlights.append(f"Level {state.get('level', 1)} Vault Dweller")
        highlights.append(f"Currently at {state.get('map_name', 'Unknown')}")
        
        # Get quest highlights from quest database
        quest_gvars = state.get('quests', {})
        if quest_gvars:
            from quest_database import get_quest_highlights
            important_quests = get_quest_highlights(quest_gvars)
            
            # Add top active quest
            if important_quests:
                top_quest = important_quests[0]
                if top_quest.get('status') == 'timer':
                    highlights.append(f"{top_quest['name']}: {top_quest.get('days_remaining', '?')} days")
                else:
                    highlights.append(f"Active: {top_quest['name']}")
        else:
            # Fallback if no quest data
            highlights.append("Quest: Find the Water Chip")
        
        # HP status
        highlights.append(f"HP: {state.get('hit_points', 0)}/{state.get('max_hit_points', 0)}")
        
        # Combat stats if available
        if state.get('total_kills', 0) > 0:
            highlights.append(f"Total Kills: {state['total_kills']}")
        
        return highlights[:5]  # Limit to 5 for stream readability
    
    def save_extended_data(self, output_path: str = None):
        """Generate and save extended character data"""
        if output_path is None:
            output_path = self.game_data_dir / "character_extended.json"
        
        extended_data = self.generate_extended_data()
        
        with open(output_path, 'w') as f:
            json.dump(extended_data, f, indent=2)
        
        print(f"Extended character data saved to {output_path}")
        return extended_data


if __name__ == "__main__":
    import argparse
    
    parser = argparse.ArgumentParser(description="Generate extended character data")
    parser.add_argument("--game-dir", default="../..", help="Path to game directory")
    parser.add_argument("--output", default=None, help="Output file path")
    
    args = parser.parse_args()
    
    generator = CharacterDataGenerator(args.game_dir)
    generator.save_extended_data(args.output)
