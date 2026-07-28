"""
Profile Manager Module

Handles creation, reading, updating, and deletion of character profiles.
Profiles are stored as JSON files in the profiles directory.
"""

import json
import os
from pathlib import Path
import logging
from datetime import datetime
import uuid

logger = logging.getLogger(__name__)


class ProfileManager:
    """Manager for character profiles"""
    
    def __init__(self, profiles_dir):
        """Initialize profile manager"""
        self.profiles_dir = Path(profiles_dir)
        
        # Create profiles directory if it doesn't exist
        self.profiles_dir.mkdir(parents=True, exist_ok=True)
        
        # Create vault-personnel subdirectory
        self.vault_personnel_dir = self.profiles_dir / "vault-personnel"
        self.vault_personnel_dir.mkdir(exist_ok=True)
        
        logger.info(f"ProfileManager initialized with directory: {self.profiles_dir}")
    
    def list_profiles(self):
        """List all available profiles"""
        try:
            profiles = []
            
            # Get all JSON files in profiles directory
            for profile_file in self.profiles_dir.glob("*.json"):
                if profile_file.name == "template.json":
                    continue  # Skip template
                
                try:
                    with open(profile_file, 'r') as f:
                        profile = json.load(f)
                    
                    profiles.append({
                        "id": profile.get("id", profile_file.stem),
                        "name": profile.get("name", "Unknown"),
                        "role": profile.get("role", "Unknown"),
                        "vault": profile.get("vault", "Unknown"),
                        "category": "custom"
                    })
                except Exception as e:
                    logger.error(f"Error reading profile {profile_file}: {e}")
            
            # Get vault personnel profiles
            for profile_file in self.vault_personnel_dir.glob("*.json"):
                try:
                    with open(profile_file, 'r') as f:
                        profile = json.load(f)
                    
                    profiles.append({
                        "id": profile.get("id", profile_file.stem),
                        "name": profile.get("name", "Unknown"),
                        "role": profile.get("role", "Unknown"),
                        "vault": profile.get("vault", "Unknown"),
                        "category": "vault-personnel"
                    })
                except Exception as e:
                    logger.error(f"Error reading vault personnel profile {profile_file}: {e}")
            
            return profiles
        except Exception as e:
            logger.error(f"Error listing profiles: {e}")
            return []
    
    def get_profile(self, profile_id):
        """Get a specific profile by ID"""
        try:
            # Try custom profiles first
            profile_file = self.profiles_dir / f"{profile_id}.json"
            
            if not profile_file.exists():
                # Try vault personnel
                profile_file = self.vault_personnel_dir / f"{profile_id}.json"
            
            if not profile_file.exists():
                logger.warning(f"Profile not found: {profile_id}")
                return None
            
            with open(profile_file, 'r') as f:
                profile = json.load(f)
            
            return profile
        except Exception as e:
            logger.error(f"Error getting profile {profile_id}: {e}")
            return None
    
    def create_profile(self, profile_data):
        """Create a new profile"""
        try:
            # Generate ID if not provided
            if 'id' not in profile_data:
                profile_data['id'] = str(uuid.uuid4())
            
            profile_id = profile_data['id']
            
            # Add metadata
            profile_data['created_at'] = datetime.now().isoformat()
            profile_data['updated_at'] = datetime.now().isoformat()
            
            # Write to file
            profile_file = self.profiles_dir / f"{profile_id}.json"
            
            with open(profile_file, 'w') as f:
                json.dump(profile_data, f, indent=2)
            
            logger.info(f"Created profile: {profile_id}")
            return profile_id
        except Exception as e:
            logger.error(f"Error creating profile: {e}")
            raise
    
    def update_profile(self, profile_id, profile_data):
        """Update an existing profile"""
        try:
            profile_file = self.profiles_dir / f"{profile_id}.json"
            
            if not profile_file.exists():
                logger.warning(f"Profile not found for update: {profile_id}")
                return False
            
            # Preserve creation date
            existing_profile = self.get_profile(profile_id)
            if existing_profile and 'created_at' in existing_profile:
                profile_data['created_at'] = existing_profile['created_at']
            
            # Update metadata
            profile_data['id'] = profile_id
            profile_data['updated_at'] = datetime.now().isoformat()
            
            # Write to file
            with open(profile_file, 'w') as f:
                json.dump(profile_data, f, indent=2)
            
            logger.info(f"Updated profile: {profile_id}")
            return True
        except Exception as e:
            logger.error(f"Error updating profile {profile_id}: {e}")
            raise
    
    def delete_profile(self, profile_id):
        """Delete a profile"""
        try:
            profile_file = self.profiles_dir / f"{profile_id}.json"
            
            if not profile_file.exists():
                logger.warning(f"Profile not found for deletion: {profile_id}")
                return False
            
            profile_file.unlink()
            logger.info(f"Deleted profile: {profile_id}")
            return True
        except Exception as e:
            logger.error(f"Error deleting profile {profile_id}: {e}")
            raise
