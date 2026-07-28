"""
Pip-Boy 2000 Web Backend Server

Flask server that provides REST API and WebSocket connections
for the Pip-Boy web interface to communicate with Fallout 1.
"""

from flask import Flask, jsonify, request, send_from_directory
from flask_cors import CORS
from flask_socketio import SocketIO, emit
import json
import os
from pathlib import Path
import logging
from datetime import datetime
import time

# Import custom modules
from game_bridge import GameBridge
from profile_manager import ProfileManager

# Setup logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

# Initialize Flask app
app = Flask(__name__)
CORS(app)
socketio = SocketIO(app, cors_allowed_origins="*")

# Load configuration
CONFIG_PATH = Path(__file__).parent / "config.json"
with open(CONFIG_PATH, 'r') as f:
    config = json.load(f)

# Initialize game bridge and profile manager
game_bridge = GameBridge(config)
profile_manager = ProfileManager(config['profiles_dir'])

# Track connected clients
connected_clients = set()

# ============================================================================
# GAME STATE ENDPOINTS
# ============================================================================

@app.route('/api/status', methods=['GET'])
def get_status():
    """Get current character status (HP, AP, location, etc.)"""
    try:
        state = game_bridge.get_game_state()
        if not state:
            return jsonify({"error": "No game data available"}), 404
        
        status = {
            "hp": state.get("hp_current", 0),
            "maxHp": state.get("hp_max", 100),
            "ap": state.get("ap_current", 0),
            "maxAp": state.get("ap_max", 10),
            "level": state.get("level", 1),
            "experience": state.get("experience", 0),
            "location": state.get("location", "Unknown"),
            "inCombat": state.get("in_combat", False),
            "radiation": state.get("radiation", 0),
            "poisoned": state.get("poisoned", False)
        }
        
        return jsonify(status)
    except Exception as e:
        logger.error(f"Error getting status: {e}")
        return jsonify({"error": str(e)}), 500

@app.route('/api/stats', methods=['GET'])
def get_stats():
    """Get SPECIAL stats, skills, perks, and traits"""
    try:
        state = game_bridge.get_game_state()
        character_data = game_bridge.get_character_data()
        
        if not state and not character_data:
            return jsonify({"error": "No character data available"}), 404
        
        # Combine data from both sources
        data = character_data if character_data else state
        
        stats = {
            "special": data.get("special", {}),
            "skills": data.get("skills", []),
            "perks": data.get("perks", []),
            "traits": data.get("traits", []),
            "level": data.get("level", 1),
            "experience": data.get("experience", 0),
            "armorClass": data.get("armor_class", 0),
            "sequence": data.get("sequence", 0),
            "healingRate": data.get("healing_rate", 0),
            "criticalChance": data.get("critical_chance", 0)
        }
        
        return jsonify(stats)
    except Exception as e:
        logger.error(f"Error getting stats: {e}")
        return jsonify({"error": str(e)}), 500

@app.route('/api/inventory', methods=['GET'])
def get_inventory():
    """Get current inventory"""
    try:
        state = game_bridge.get_game_state()
        if not state:
            return jsonify({"error": "No game data available"}), 404
        
        inventory = {
            "equipped": state.get("equipped", []),
            "items": state.get("inventory", []),
            "weight": state.get("weight_current", 0),
            "maxWeight": state.get("weight_max", 100)
        }
        
        return jsonify(inventory)
    except Exception as e:
        logger.error(f"Error getting inventory: {e}")
        return jsonify({"error": str(e)}), 500

@app.route('/api/quests', methods=['GET'])
def get_quests():
    """Get quest log"""
    try:
        state = game_bridge.get_game_state()
        if not state:
            return jsonify({"error": "No game data available"}), 404
        
        quests = state.get("quests", [])
        return jsonify(quests)
    except Exception as e:
        logger.error(f"Error getting quests: {e}")
        return jsonify({"error": str(e)}), 500

@app.route('/api/world-map', methods=['GET'])
def get_world_map():
    """Get world map data with discovered locations"""
    try:
        state = game_bridge.get_game_state()
        if not state:
            return jsonify({"error": "No game data available"}), 404
        
        world_map = {
            "currentLocation": state.get("location", "Unknown"),
            "locations": state.get("discovered_locations", []),
            "routes": state.get("travel_routes", [])
        }
        
        return jsonify(world_map)
    except Exception as e:
        logger.error(f"Error getting world map: {e}")
        return jsonify({"error": str(e)}), 500

# ============================================================================
# GAME CONTROL ENDPOINTS
# ============================================================================

@app.route('/api/inventory/use', methods=['POST'])
def use_item():
    """Use an item from inventory"""
    try:
        data = request.json
        item_id = data.get('itemId')
        
        if not item_id:
            return jsonify({"error": "itemId is required"}), 400
        
        result = game_bridge.send_action({
            "action": "use_item",
            "item_id": item_id,
            "timestamp": datetime.now().isoformat()
        })
        
        return jsonify({"success": True, "result": result})
    except Exception as e:
        logger.error(f"Error using item: {e}")
        return jsonify({"error": str(e)}), 500

@app.route('/api/inventory/equip', methods=['POST'])
def equip_item():
    """Equip a weapon or armor"""
    try:
        data = request.json
        item_id = data.get('itemId')
        slot = data.get('slot', 'weapon')
        
        if not item_id:
            return jsonify({"error": "itemId is required"}), 400
        
        result = game_bridge.send_action({
            "action": "equip_item",
            "item_id": item_id,
            "slot": slot,
            "timestamp": datetime.now().isoformat()
        })
        
        return jsonify({"success": True, "result": result})
    except Exception as e:
        logger.error(f"Error equipping item: {e}")
        return jsonify({"error": str(e)}), 500

@app.route('/api/inventory/drop', methods=['POST'])
def drop_item():
    """Drop an item from inventory"""
    try:
        data = request.json
        item_id = data.get('itemId')
        quantity = data.get('quantity', 1)
        
        if not item_id:
            return jsonify({"error": "itemId is required"}), 400
        
        result = game_bridge.send_action({
            "action": "drop_item",
            "item_id": item_id,
            "quantity": quantity,
            "timestamp": datetime.now().isoformat()
        })
        
        return jsonify({"success": True, "result": result})
    except Exception as e:
        logger.error(f"Error dropping item: {e}")
        return jsonify({"error": str(e)}), 500

@app.route('/api/rest', methods=['POST'])
def rest():
    """Rest to heal"""
    try:
        data = request.json
        hours = data.get('hours', 1)
        
        result = game_bridge.send_action({
            "action": "rest",
            "hours": hours,
            "timestamp": datetime.now().isoformat()
        })
        
        return jsonify({"success": True, "result": result})
    except Exception as e:
        logger.error(f"Error resting: {e}")
        return jsonify({"error": str(e)}), 500

# ============================================================================
# PROFILE ENDPOINTS
# ============================================================================

@app.route('/api/profiles', methods=['GET'])
def list_profiles():
    """List all character profiles"""
    try:
        profiles = profile_manager.list_profiles()
        return jsonify(profiles)
    except Exception as e:
        logger.error(f"Error listing profiles: {e}")
        return jsonify({"error": str(e)}), 500

@app.route('/api/profiles/<profile_id>', methods=['GET'])
def get_profile(profile_id):
    """Get a specific profile"""
    try:
        profile = profile_manager.get_profile(profile_id)
        if not profile:
            return jsonify({"error": "Profile not found"}), 404
        return jsonify(profile)
    except Exception as e:
        logger.error(f"Error getting profile: {e}")
        return jsonify({"error": str(e)}), 500

@app.route('/api/profiles', methods=['POST'])
def create_profile():
    """Create a new profile"""
    try:
        data = request.json
        profile_id = profile_manager.create_profile(data)
        return jsonify({"success": True, "id": profile_id}), 201
    except Exception as e:
        logger.error(f"Error creating profile: {e}")
        return jsonify({"error": str(e)}), 500

@app.route('/api/profiles/<profile_id>', methods=['PUT'])
def update_profile(profile_id):
    """Update an existing profile"""
    try:
        data = request.json
        success = profile_manager.update_profile(profile_id, data)
        if not success:
            return jsonify({"error": "Profile not found"}), 404
        return jsonify({"success": True})
    except Exception as e:
        logger.error(f"Error updating profile: {e}")
        return jsonify({"error": str(e)}), 500

@app.route('/api/profiles/<profile_id>', methods=['DELETE'])
def delete_profile(profile_id):
    """Delete a profile"""
    try:
        success = profile_manager.delete_profile(profile_id)
        if not success:
            return jsonify({"error": "Profile not found"}), 404
        return jsonify({"success": True})
    except Exception as e:
        logger.error(f"Error deleting profile: {e}")
        return jsonify({"error": str(e)}), 500

# ============================================================================
# WEBSOCKET EVENTS
# ============================================================================

@socketio.on('connect')
def handle_connect():
    """Handle client connection"""
    logger.info(f"Client connected: {request.sid}")
    connected_clients.add(request.sid)
    emit('connection_established', {'message': 'Connected to Pip-Boy server'})

@socketio.on('disconnect')
def handle_disconnect():
    """Handle client disconnection"""
    logger.info(f"Client disconnected: {request.sid}")
    connected_clients.discard(request.sid)

@socketio.on('subscribe_updates')
def handle_subscribe():
    """Client requests real-time updates"""
    logger.info(f"Client subscribed to updates: {request.sid}")
    emit('subscribed', {'message': 'Subscribed to game updates'})

# Background task to push updates to connected clients
def push_game_updates():
    """Continuously push game state updates to connected clients"""
    while True:
        if connected_clients:
            try:
                state = game_bridge.get_game_state()
                if state:
                    socketio.emit('game_state_update', state)
            except Exception as e:
                logger.error(f"Error pushing updates: {e}")
        
        time.sleep(config['poll_interval'])

# ============================================================================
# HEALTH CHECK
# ============================================================================

@app.route('/api/health', methods=['GET'])
def health_check():
    """Health check endpoint"""
    return jsonify({
        "status": "ok",
        "server": "Pip-Boy Backend",
        "version": "1.0.0",
        "timestamp": datetime.now().isoformat(),
        "game_connected": game_bridge.is_game_connected()
    })

# ============================================================================
# MAIN
# ============================================================================

if __name__ == '__main__':
    logger.info("Starting Pip-Boy 2000 Web Backend Server")
    logger.info(f"Server running on {config['host']}:{config['port']}")
    logger.info(f"Game data path: {config['game_data_path']}")
    logger.info(f"Profiles directory: {config['profiles_dir']}")
    
    # Start background update task
    socketio.start_background_task(push_game_updates)
    
    # Run server
    socketio.run(
        app,
        host=config['host'],
        port=config['port'],
        debug=config['debug']
    )
