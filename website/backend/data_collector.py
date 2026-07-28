"""
Data Collector for Website

Continuously monitors game JSON files and stores data in SQLite database.
Runs as background service, polls every second.
"""

import json
import sqlite3
import time
import os
from datetime import datetime
from pathlib import Path
import logging

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s',
    handlers=[
        logging.FileHandler('data_collector.log'),
        logging.StreamHandler()
    ]
)

class GameDataCollector:
    """Collects data from game JSON files and stores in database"""
    
    def __init__(self, game_dir: str = "../..", db_path: str = "./database/game_data.db"):
        self.game_dir = Path(game_dir)
        self.db_path = db_path
        self.running = False
        
        # Ensure database directory exists
        os.makedirs(os.path.dirname(db_path), exist_ok=True)
        
        # Initialize database
        self.init_database()
        
        logging.info(f"Data collector initialized. Game dir: {self.game_dir}, DB: {self.db_path}")
    
    def init_database(self):
        """Create database schema if it doesn't exist"""
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        
        # Game states table - snapshots of game state
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS game_states (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
                hp_current INTEGER,
                hp_max INTEGER,
                ap_current INTEGER,
                ap_max INTEGER,
                level INTEGER,
                experience INTEGER,
                armor_class INTEGER,
                map_name TEXT,
                tile INTEGER,
                elevation INTEGER,
                in_combat BOOLEAN,
                session_time INTEGER,
                last_action TEXT,
                last_action_result TEXT
            )
        ''')
        
        # Inventory snapshots
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS inventory_snapshots (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
                item_pid INTEGER,
                item_name TEXT,
                quantity INTEGER
            )
        ''')
        
        # Events log
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS events (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
                event_type TEXT,
                event_description TEXT,
                priority TEXT
            )
        ''')
        
        # Milestones
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS milestones (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
                description TEXT,
                location TEXT
            )
        ''')
        
        # Decisions (AI memory)
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS decisions (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
                map_name TEXT,
                tile INTEGER,
                elevation INTEGER,
                action TEXT,
                target TEXT,
                result TEXT
            )
        ''')
        
        # Skills progression
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS skills (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
                skill_name TEXT,
                skill_value INTEGER
            )
        ''')
        
        # Stats progression (SPECIAL)
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS stats (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
                stat_name TEXT,
                stat_value INTEGER
            )
        ''')
        
        # Session statistics
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS session_stats (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
                total_kills INTEGER,
                total_damage INTEGER,
                session_time INTEGER
            )
        ''')
        
        # Items collected history
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS items_collected (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
                item_pid INTEGER,
                item_name TEXT,
                quantity INTEGER,
                location TEXT
            )
        ''')
        
        conn.commit()
        conn.close()
        logging.info("Database schema initialized")
    
    def collect_game_state(self):
        """Read and store current game state"""
        state_file = self.game_dir / "ai_state.json"
        
        if not state_file.exists():
            return
        
        try:
            with open(state_file, 'r') as f:
                data = json.load(f)
            
            conn = sqlite3.connect(self.db_path)
            cursor = conn.cursor()
            
            # Insert game state snapshot
            cursor.execute('''
                INSERT INTO game_states (
                    hp_current, hp_max, ap_current, ap_max, level, experience,
                    armor_class, map_name, tile, elevation, in_combat, session_time,
                    last_action, last_action_result
                ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
            ''', (
                data.get('hit_points', 0),
                data.get('max_hit_points', 0),
                data.get('action_points', 0),
                data.get('max_action_points', 0),
                data.get('level', 1),
                data.get('experience', 0),
                data.get('armor_class', 0),
                data.get('map_name', 'Unknown'),
                data.get('player_tile', 0),
                data.get('player_elevation', 0),
                data.get('in_combat', False),
                data.get('session_time_seconds', 0),
                'none',  # Would need to parse from last action
                data.get('last_action_result', 'none')
            ))
            
            # Store inventory
            inventory = data.get('inventory', [])
            for item in inventory:
                cursor.execute('''
                    INSERT INTO inventory_snapshots (item_pid, item_name, quantity)
                    VALUES (?, ?, ?)
                ''', (item.get('pid', 0), item.get('name', 'Unknown'), item.get('quantity', 1)))
            
            # Store skills
            skills = data.get('skills', [])
            for skill in skills:
                cursor.execute('''
                    INSERT INTO skills (skill_name, skill_value)
                    VALUES (?, ?)
                ''', (skill.get('name', 'Unknown'), skill.get('value', 0)))
            
            # Store session stats
            cursor.execute('''
                INSERT INTO session_stats (total_kills, total_damage, session_time)
                VALUES (?, ?, ?)
            ''', (
                data.get('total_kills', 0),
                data.get('total_damage_dealt', 0),
                data.get('session_time_seconds', 0)
            ))
            
            conn.commit()
            conn.close()
            
        except Exception as e:
            logging.error(f"Error collecting game state: {e}")
    
    def collect_character_data(self):
        """Read and store character data"""
        char_file = self.game_dir / "character_data.json"
        
        if not char_file.exists():
            return
        
        try:
            with open(char_file, 'r') as f:
                data = json.load(f)
            
            conn = sqlite3.connect(self.db_path)
            cursor = conn.cursor()
            
            # Store items collected history
            items_collected = data.get('items_collected', [])
            for item in items_collected:
                # Check if already stored
                cursor.execute('''
                    SELECT COUNT(*) FROM items_collected 
                    WHERE item_pid = ? AND timestamp = ?
                ''', (item.get('pid', 0), item.get('timestamp', 0)))
                
                if cursor.fetchone()[0] == 0:
                    cursor.execute('''
                        INSERT INTO items_collected (timestamp, item_pid, item_name, quantity, location)
                        VALUES (datetime(?, 'unixepoch'), ?, ?, ?, ?)
                    ''', (
                        item.get('timestamp', 0),
                        item.get('pid', 0),
                        item.get('name', 'Unknown'),
                        item.get('quantity', 1),
                        item.get('location', 'Unknown')
                    ))
            
            # Store milestones
            milestones = data.get('milestones', [])
            for milestone in milestones:
                cursor.execute('''
                    SELECT COUNT(*) FROM milestones 
                    WHERE description = ? AND timestamp = ?
                ''', (milestone.get('description', ''), milestone.get('timestamp', 0)))
                
                if cursor.fetchone()[0] == 0:
                    cursor.execute('''
                        INSERT INTO milestones (timestamp, description, location)
                        VALUES (datetime(?, 'unixepoch'), ?, ?)
                    ''', (
                        milestone.get('timestamp', 0),
                        milestone.get('description', ''),
                        milestone.get('location', 'Unknown')
                    ))
            
            conn.commit()
            conn.close()
            
        except Exception as e:
            logging.error(f"Error collecting character data: {e}")
    
    def collect_memory(self):
        """Read and store AI memory"""
        memory_file = self.game_dir / "ai_memory.json"
        
        if not memory_file.exists():
            return
        
        try:
            with open(memory_file, 'r') as f:
                data = json.load(f)
            
            conn = sqlite3.connect(self.db_path)
            cursor = conn.cursor()
            
            memories = data.get('memories', [])
            for memory in memories:
                # Check if already stored
                cursor.execute('''
                    SELECT COUNT(*) FROM decisions 
                    WHERE timestamp = ? AND action = ?
                ''', (memory.get('timestamp', 0), memory.get('action', '')))
                
                if cursor.fetchone()[0] == 0:
                    cursor.execute('''
                        INSERT INTO decisions (timestamp, map_name, tile, elevation, action, target, result)
                        VALUES (datetime(?, 'unixepoch'), ?, ?, ?, ?, ?, ?)
                    ''', (
                        memory.get('timestamp', 0),
                        memory.get('map', 'Unknown'),
                        memory.get('tile', 0),
                        memory.get('elevation', 0),
                        memory.get('action', ''),
                        memory.get('target', ''),
                        memory.get('result', '')
                    ))
            
            conn.commit()
            conn.close()
            
        except Exception as e:
            logging.error(f"Error collecting memory: {e}")
    
    def run(self, interval: float = 1.0):
        """Main collection loop"""
        self.running = True
        logging.info(f"Starting data collection (interval: {interval}s)")
        
        while self.running:
            try:
                self.collect_game_state()
                self.collect_character_data()
                self.collect_memory()
                time.sleep(interval)
            except KeyboardInterrupt:
                logging.info("Stopping data collector...")
                self.running = False
            except Exception as e:
                logging.error(f"Error in collection loop: {e}")
                time.sleep(interval)
    
    def stop(self):
        """Stop the collector"""
        self.running = False


if __name__ == "__main__":
    import argparse
    
    parser = argparse.ArgumentParser(description="Fallout 1 Game Data Collector")
    parser.add_argument("--game-dir", default="../..", help="Path to game directory")
    parser.add_argument("--db-path", default="./database/game_data.db", help="Path to SQLite database")
    parser.add_argument("--interval", type=float, default=1.0, help="Collection interval in seconds")
    
    args = parser.parse_args()
    
    collector = GameDataCollector(args.game_dir, args.db_path)
    collector.run(args.interval)
