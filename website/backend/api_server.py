"""
API Server for Website

Serves collected data from SQLite database to frontend.
RESTful API endpoints for game state, history, statistics.
"""

from flask import Flask, jsonify, request
from flask_cors import CORS
import sqlite3
import json
from datetime import datetime, timedelta
from pathlib import Path
from character_data_generator import CharacterDataGenerator

app = Flask(__name__)
CORS(app)  # Enable CORS for frontend

DB_PATH = "./database/game_data.db"
GAME_DATA_DIR = Path("../..")

def get_db():
    """Get database connection"""
    conn = sqlite3.connect(DB_PATH)
    conn.row_factory = sqlite3.Row  # Return rows as dictionaries
    return conn

@app.route('/api/current-state', methods=['GET'])
def get_current_state():
    """Get most recent game state"""
    conn = get_db()
    cursor = conn.cursor()
    
    # Get latest state
    cursor.execute('''
        SELECT * FROM game_states 
        ORDER BY timestamp DESC 
        LIMIT 1
    ''')
    state = cursor.fetchone()
    
    if not state:
        return jsonify({"error": "No data available"}), 404
    
    # Get latest inventory
    cursor.execute('''
        SELECT DISTINCT item_name, item_pid, quantity 
        FROM inventory_snapshots 
        WHERE timestamp >= (SELECT timestamp FROM game_states ORDER BY timestamp DESC LIMIT 1)
    ''')
    inventory = [dict(row) for row in cursor.fetchall()]
    
    # Get latest session stats
    cursor.execute('''
        SELECT * FROM session_stats 
        ORDER BY timestamp DESC 
        LIMIT 1
    ''')
    session = cursor.fetchone()
    
    conn.close()
    
    return jsonify({
        "state": dict(state) if state else {},
        "inventory": inventory,
        "session": dict(session) if session else {}
    })

@app.route('/api/stats-history', methods=['GET'])
def get_stats_history():
    """Get character stats over time"""
    hours = request.args.get('hours', 1, type=int)
    
    conn = get_db()
    cursor = conn.cursor()
    
    cursor.execute('''
        SELECT timestamp, hp_current, hp_max, level, experience, armor_class
        FROM game_states 
        WHERE timestamp >= datetime('now', '-' || ? || ' hours')
        ORDER BY timestamp ASC
    ''', (hours,))
    
    history = [dict(row) for row in cursor.fetchall()]
    conn.close()
    
    return jsonify(history)

@app.route('/api/skills-current', methods=['GET'])
def get_current_skills():
    """Get current skill levels"""
    conn = get_db()
    cursor = conn.cursor()
    
    # Get most recent skills
    cursor.execute('''
        SELECT skill_name, skill_value, timestamp
        FROM skills 
        WHERE timestamp = (SELECT MAX(timestamp) FROM skills)
    ''')
    
    skills = [dict(row) for row in cursor.fetchall()]
    conn.close()
    
    return jsonify(skills)

@app.route('/api/milestones', methods=['GET'])
def get_milestones():
    """Get all milestones"""
    conn = get_db()
    cursor = conn.cursor()
    
    cursor.execute('''
        SELECT * FROM milestones 
        ORDER BY timestamp DESC
    ''')
    
    milestones = [dict(row) for row in cursor.fetchall()]
    conn.close()
    
    return jsonify(milestones)

@app.route('/api/items-collected', methods=['GET'])
def get_items_collected():
    """Get history of items collected"""
    limit = request.args.get('limit', 100, type=int)
    
    conn = get_db()
    cursor = conn.cursor()
    
    cursor.execute('''
        SELECT * FROM items_collected 
        ORDER BY timestamp DESC
        LIMIT ?
    ''', (limit,))
    
    items = [dict(row) for row in cursor.fetchall()]
    conn.close()
    
    return jsonify(items)

@app.route('/api/decisions', methods=['GET'])
def get_decisions():
    """Get AI decision history"""
    limit = request.args.get('limit', 50, type=int)
    
    conn = get_db()
    cursor = conn.cursor()
    
    cursor.execute('''
        SELECT * FROM decisions 
        ORDER BY timestamp DESC
        LIMIT ?
    ''', (limit,))
    
    decisions = [dict(row) for row in cursor.fetchall()]
    conn.close()
    
    return jsonify(decisions)

@app.route('/api/session-stats', methods=['GET'])
def get_session_stats():
    """Get aggregated session statistics"""
    conn = get_db()
    cursor = conn.cursor()
    
    # Get latest session
    cursor.execute('''
        SELECT * FROM session_stats 
        ORDER BY timestamp DESC 
        LIMIT 1
    ''')
    current_session = cursor.fetchone()
    
    # Get progression over time
    cursor.execute('''
        SELECT 
            COUNT(DISTINCT DATE(timestamp)) as days_played,
            MAX(level) as max_level,
            MAX(experience) as total_xp,
            COUNT(*) as total_snapshots
        FROM game_states
    ''')
    aggregated = cursor.fetchone()
    
    # Get kill count progression
    cursor.execute('''
        SELECT timestamp, total_kills 
        FROM session_stats 
        ORDER BY timestamp ASC
    ''')
    kill_history = [dict(row) for row in cursor.fetchall()]
    
    conn.close()
    
    return jsonify({
        "current": dict(current_session) if current_session else {},
        "aggregated": dict(aggregated) if aggregated else {},
        "kill_history": kill_history
    })

@app.route('/api/location-history', methods=['GET'])
def get_location_history():
    """Get visited locations"""
    conn = get_db()
    cursor = conn.cursor()
    
    cursor.execute('''
        SELECT DISTINCT map_name, MIN(timestamp) as first_visit, MAX(timestamp) as last_visit
        FROM game_states
        GROUP BY map_name
        ORDER BY first_visit ASC
    ''')
    
    locations = [dict(row) for row in cursor.fetchall()]
    conn.close()
    
    return jsonify(locations)

@app.route('/api/combat-stats', methods=['GET'])
def get_combat_stats():
    """Get combat statistics"""
    conn = get_db()
    cursor = conn.cursor()
    
    # Combat encounters
    cursor.execute('''
        SELECT COUNT(*) as combat_encounters
        FROM game_states
        WHERE in_combat = 1
    ''')
    encounters = cursor.fetchone()
    
    # Average HP during combat
    cursor.execute('''
        SELECT AVG(hp_current * 100.0 / hp_max) as avg_hp_percent
        FROM game_states
        WHERE in_combat = 1 AND hp_max > 0
    ''')
    avg_hp = cursor.fetchone()
    
    # Latest kills/damage
    cursor.execute('''
        SELECT total_kills, total_damage
        FROM session_stats
        ORDER BY timestamp DESC
        LIMIT 1
    ''')
    latest = cursor.fetchone()
    
    conn.close()
    
    return jsonify({
        "encounters": dict(encounters) if encounters else {},
        "avg_hp_in_combat": dict(avg_hp) if avg_hp else {},
        "latest": dict(latest) if latest else {}
    })

@app.route('/api/health', methods=['GET'])
def health_check():
    """Health check endpoint"""
    try:
        conn = get_db()
        cursor = conn.cursor()
        cursor.execute('SELECT COUNT(*) FROM game_states')
        count = cursor.fetchone()[0]
        conn.close()
        
        return jsonify({
            "status": "healthy",
            "database": "connected",
            "total_states": count
        })
    except Exception as e:
        return jsonify({
            "status": "unhealthy",
            "error": str(e)
        }), 500

@app.route('/api/character-extended', methods=['GET'])
def get_character_extended():
    """Get extended character data for terminal UI"""
    try:
        # Check for cached extended data file
        extended_file = GAME_DATA_DIR / "character_extended.json"
        
        if extended_file.exists():
            # Return cached data if recent (less than 10 seconds old)
            import os
            if (datetime.now().timestamp() - os.path.getmtime(extended_file)) < 10:
                with open(extended_file, 'r') as f:
                    return jsonify(json.load(f))
        
        # Generate new extended data
        generator = CharacterDataGenerator(str(GAME_DATA_DIR))
        extended_data = generator.generate_extended_data()
        
        # Cache it
        with open(extended_file, 'w') as f:
            json.dump(extended_data, f, indent=2)
        
        return jsonify(extended_data)
    
    except Exception as e:
        return jsonify({
            "error": str(e),
            "message": "Error generating extended character data"
        }), 500

@app.route('/api/timeline', methods=['GET'])
def get_timeline():
    """Get timeline events"""
    try:
        generator = CharacterDataGenerator(str(GAME_DATA_DIR))
        base_data = generator.load_base_game_data()
        timeline = generator._generate_timeline(base_data)
        return jsonify(timeline)
    except Exception as e:
        return jsonify({"error": str(e)}), 500

@app.route('/api/quests', methods=['GET'])
def get_quests():
    """Get quest log"""
    try:
        generator = CharacterDataGenerator(str(GAME_DATA_DIR))
        base_data = generator.load_base_game_data()
        quests = generator._generate_quests(base_data)
        return jsonify({"quests": quests})
    except Exception as e:
        return jsonify({"error": str(e)}), 500

@app.route('/api/locations-extended', methods=['GET'])
def get_locations_extended():
    """Get extended location data"""
    try:
        generator = CharacterDataGenerator(str(GAME_DATA_DIR))
        base_data = generator.load_base_game_data()
        locations = generator._generate_locations(base_data)
        map_data = generator._generate_map_data(base_data)
        return jsonify({
            "locations": locations,
            "map": map_data
        })
    except Exception as e:
        return jsonify({"error": str(e)}), 500

@app.route('/api/journal', methods=['GET'])
def get_journal():
    """Get journal entries"""
    try:
        generator = CharacterDataGenerator(str(GAME_DATA_DIR))
        base_data = generator.load_base_game_data()
        journal = generator._generate_journal(base_data)
        return jsonify({"journal": journal})
    except Exception as e:
        return jsonify({"error": str(e)}), 500

if __name__ == '__main__':
    import argparse
    
    parser = argparse.ArgumentParser(description="Fallout 1 Website API Server")
    parser.add_argument("--port", type=int, default=5000, help="Port to run server on")
    parser.add_argument("--host", default="0.0.0.0", help="Host to bind to")
    parser.add_argument("--debug", action="store_true", help="Run in debug mode")
    
    args = parser.parse_args()
    
    print(f"Starting API server on {args.host}:{args.port}")
    app.run(host=args.host, port=args.port, debug=args.debug)
