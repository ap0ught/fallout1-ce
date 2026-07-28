import React, { useState, useEffect } from 'react'

function ProfileBrowser() {
  const [profiles, setProfiles] = useState([])
  const [selectedProfile, setSelectedProfile] = useState(null)
  const [isCreating, setIsCreating] = useState(false)
  const [newProfile, setNewProfile] = useState(getEmptyProfile())

  useEffect(() => {
    fetchProfiles()
  }, [])

  const fetchProfiles = async () => {
    try {
      const response = await fetch('/api/profiles')
      if (response.ok) {
        const data = await response.json()
        setProfiles(data)
      }
    } catch (error) {
      console.error('Error fetching profiles:', error)
    }
  }

  const fetchProfileDetails = async (profileId) => {
    try {
      const response = await fetch(`/api/profiles/${profileId}`)
      if (response.ok) {
        const data = await response.json()
        setSelectedProfile(data)
      }
    } catch (error) {
      console.error('Error fetching profile:', error)
    }
  }

  const handleCreateProfile = async () => {
    try {
      const response = await fetch('/api/profiles', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(newProfile)
      })
      
      if (response.ok) {
        setIsCreating(false)
        setNewProfile(getEmptyProfile())
        fetchProfiles()
      }
    } catch (error) {
      console.error('Error creating profile:', error)
    }
  }

  const handleDeleteProfile = async (profileId) => {
    if (!confirm('Are you sure you want to delete this profile?')) {
      return
    }

    try {
      const response = await fetch(`/api/profiles/${profileId}`, {
        method: 'DELETE'
      })
      
      if (response.ok) {
        setSelectedProfile(null)
        fetchProfiles()
      }
    } catch (error) {
      console.error('Error deleting profile:', error)
    }
  }

  return (
    <div className="profile-browser">
      <div className="profile-sidebar">
        <div className="sidebar-header">
          <h2>Character Profiles</h2>
          <button onClick={() => setIsCreating(!isCreating)}>
            {isCreating ? 'Cancel' : '+ New Profile'}
          </button>
        </div>

        <div className="profile-categories">
          <h3>Vault Personnel</h3>
          <div className="profile-list">
            {profiles
              .filter(p => p.category === 'vault-personnel')
              .map(profile => (
                <div 
                  key={profile.id} 
                  className="profile-item"
                  onClick={() => fetchProfileDetails(profile.id)}
                >
                  <span className="profile-name">{profile.name}</span>
                  <span className="profile-role">{profile.role}</span>
                </div>
              ))}
          </div>

          <h3>Custom Characters</h3>
          <div className="profile-list">
            {profiles
              .filter(p => p.category === 'custom')
              .map(profile => (
                <div 
                  key={profile.id} 
                  className="profile-item"
                  onClick={() => fetchProfileDetails(profile.id)}
                >
                  <span className="profile-name">{profile.name}</span>
                  <span className="profile-role">{profile.role}</span>
                </div>
              ))}
          </div>
        </div>
      </div>

      <div className="profile-content">
        {isCreating && (
          <div className="profile-editor">
            <h2>Create New Profile</h2>
            <ProfileForm 
              profile={newProfile} 
              onChange={setNewProfile}
              onSave={handleCreateProfile}
              onCancel={() => setIsCreating(false)}
            />
          </div>
        )}

        {!isCreating && selectedProfile && (
          <div className="profile-details">
            <ProfileDetails 
              profile={selectedProfile}
              onDelete={() => handleDeleteProfile(selectedProfile.id)}
            />
          </div>
        )}

        {!isCreating && !selectedProfile && (
          <div className="profile-empty">
            <h2>Vault Personnel Database</h2>
            <p>Select a profile from the list or create a new character.</p>
            <p>This system allows you to browse Vault 13 personnel and create custom character builds.</p>
          </div>
        )}
      </div>
    </div>
  )
}

function ProfileDetails({ profile, onDelete }) {
  return (
    <div className="profile-view">
      <div className="profile-header">
        <h1>{profile.name}</h1>
        <p className="profile-tagline">{profile.tagline}</p>
      </div>

      <div className="profile-info">
        <div className="info-grid">
          <div><strong>Age:</strong> {profile.age}</div>
          <div><strong>Gender:</strong> {profile.gender}</div>
          <div><strong>Role:</strong> {profile.role}</div>
          <div><strong>Vault:</strong> {profile.vault}</div>
        </div>

        <div className="profile-background">
          <h3>Background</h3>
          <p>{profile.background}</p>
        </div>

        {profile.description && (
          <div className="profile-description">
            <h3>Description</h3>
            <p>{profile.description}</p>
          </div>
        )}
      </div>

      <div className="profile-special">
        <h3>S.P.E.C.I.A.L.</h3>
        <div className="special-grid">
          <div>Strength: {profile.special?.strength}</div>
          <div>Perception: {profile.special?.perception}</div>
          <div>Endurance: {profile.special?.endurance}</div>
          <div>Charisma: {profile.special?.charisma}</div>
          <div>Intelligence: {profile.special?.intelligence}</div>
          <div>Agility: {profile.special?.agility}</div>
          <div>Luck: {profile.special?.luck}</div>
        </div>
      </div>

      <div className="profile-skills">
        <h3>Tag Skills</h3>
        <ul>
          {profile.tagSkills && profile.tagSkills.map((skill, i) => (
            <li key={i}>{skill}</li>
          ))}
        </ul>
      </div>

      <div className="profile-traits">
        <h3>Traits</h3>
        <ul>
          {profile.traits && profile.traits.map((trait, i) => (
            <li key={i}>{typeof trait === 'string' ? trait : trait.name}</li>
          ))}
        </ul>
      </div>

      {profile.relationships && profile.relationships.length > 0 && (
        <div className="profile-relationships">
          <h3>Relationships</h3>
          {profile.relationships.map((rel, i) => (
            <div key={i} className="relationship-item">
              <strong>{rel.name}</strong> ({rel.relationship})
              <p>{rel.description}</p>
            </div>
          ))}
        </div>
      )}

      <div className="profile-actions">
        <button className="delete-btn" onClick={onDelete}>Delete Profile</button>
      </div>
    </div>
  )
}

function ProfileForm({ profile, onChange, onSave, onCancel }) {
  const updateField = (field, value) => {
    onChange({ ...profile, [field]: value })
  }

  const updateSpecial = (stat, value) => {
    onChange({
      ...profile,
      special: { ...profile.special, [stat]: parseInt(value) }
    })
  }

  return (
    <div className="profile-form">
      <div className="form-group">
        <label>Name</label>
        <input 
          type="text" 
          value={profile.name} 
          onChange={(e) => updateField('name', e.target.value)}
        />
      </div>

      <div className="form-row">
        <div className="form-group">
          <label>Age</label>
          <input 
            type="number" 
            value={profile.age} 
            onChange={(e) => updateField('age', parseInt(e.target.value))}
          />
        </div>
        <div className="form-group">
          <label>Gender</label>
          <input 
            type="text" 
            value={profile.gender} 
            onChange={(e) => updateField('gender', e.target.value)}
          />
        </div>
      </div>

      <div className="form-group">
        <label>Role</label>
        <input 
          type="text" 
          value={profile.role} 
          onChange={(e) => updateField('role', e.target.value)}
        />
      </div>

      <div className="form-group">
        <label>Vault</label>
        <input 
          type="text" 
          value={profile.vault} 
          onChange={(e) => updateField('vault', e.target.value)}
        />
      </div>

      <div className="form-group">
        <label>Tagline</label>
        <input 
          type="text" 
          value={profile.tagline} 
          onChange={(e) => updateField('tagline', e.target.value)}
        />
      </div>

      <div className="form-group">
        <label>Background</label>
        <textarea 
          value={profile.background} 
          onChange={(e) => updateField('background', e.target.value)}
          rows="4"
        />
      </div>

      <div className="special-form">
        <h3>S.P.E.C.I.A.L.</h3>
        {['strength', 'perception', 'endurance', 'charisma', 'intelligence', 'agility', 'luck'].map(stat => (
          <div key={stat} className="form-group">
            <label>{stat.charAt(0).toUpperCase() + stat.slice(1)}</label>
            <input 
              type="number" 
              min="1" 
              max="10" 
              value={profile.special[stat]} 
              onChange={(e) => updateSpecial(stat, e.target.value)}
            />
          </div>
        ))}
      </div>

      <div className="form-actions">
        <button onClick={onSave}>Save Profile</button>
        <button onClick={onCancel}>Cancel</button>
      </div>
    </div>
  )
}

function getEmptyProfile() {
  return {
    name: '',
    age: 25,
    gender: 'Male',
    role: 'Vault Dweller',
    vault: 'Vault 13',
    background: '',
    tagline: '',
    special: {
      strength: 5,
      perception: 5,
      endurance: 5,
      charisma: 5,
      intelligence: 5,
      agility: 5,
      luck: 5
    },
    tagSkills: [],
    traits: []
  }
}

export default ProfileBrowser
