"""
Game Bridge Module

Handles communication between the Pip-Boy web interface and Fallout 1.
Reads game state from JSON files and sends commands back to the game.
"""

import json
import os
from pathlib import Path
import logging
from datetime import datetime

logger = logging.getLogger(__name__)


class GameBridge:
    """Bridge between web interface and game"""
    
    def __init__(self, config):
        """Initialize game bridge with configuration"""
        self.config = config
        self.game_data_path = Path(config['game_data_path'])
        self.ai_state_file = self.game_data_path / config['ai_state_file']
        self.ai_action_file = self.game_data_path / config['ai_action_file']
        self.character_data_file = self.game_data_path / config['character_data_file']
        
        logger.info(f"GameBridge initialized")
        logger.info(f"Watching for game state at: {self.ai_state_file}")
        logger.info(f"Will write actions to: {self.ai_action_file}")
    
    def is_game_connected(self):
        """Check if game is running and exporting data"""
        try:
            if not self.ai_state_file.exists():
                return False
            
            # Check if file was modified recently (within last 10 seconds)
            mtime = os.path.getmtime(self.ai_state_file)
            age = datetime.now().timestamp() - mtime
            
            return age < 10.0
        except Exception as e:
            logger.error(f"Error checking game connection: {e}")
            return False
    
    def get_game_state(self):
        """Read current game state from JSON file"""
        try:
            if not self.ai_state_file.exists():
                logger.warning(f"Game state file not found: {self.ai_state_file}")
                return None
            
            with open(self.ai_state_file, 'r') as f:
                state = json.load(f)
            
            return state
        except json.JSONDecodeError as e:
            logger.error(f"Invalid JSON in game state file: {e}")
            return None
        except Exception as e:
            logger.error(f"Error reading game state: {e}")
            return None
    
    def get_character_data(self):
        """Read extended character data if available"""
        try:
            if not self.character_data_file.exists():
                logger.debug(f"Character data file not found: {self.character_data_file}")
                return None
            
            with open(self.character_data_file, 'r') as f:
                data = json.load(f)
            
            return data
        except Exception as e:
            logger.error(f"Error reading character data: {e}")
            return None
    
    def send_action(self, action_data):
        """Send an action command to the game"""
        try:
            # Add metadata
            action_data['source'] = 'pipboy-web'
            action_data['timestamp'] = datetime.now().isoformat()
            
            # Write to action file
            with open(self.ai_action_file, 'w') as f:
                json.dump(action_data, f, indent=2)
            
            logger.info(f"Sent action to game: {action_data.get('action', 'unknown')}")
            return {"status": "queued", "action": action_data['action']}
        except Exception as e:
            logger.error(f"Error sending action: {e}")
            raise
    
    def read_action_result(self):
        """Read the result of a previous action (if game writes results)"""
        try:
            # This would read from a hypothetical ai_action_result.json
            # For now, actions are fire-and-forget
            return None
        except Exception as e:
            logger.error(f"Error reading action result: {e}")
            return None
