import React, { useState } from 'react'
import '../../styles/panels/Locations.css'

function Locations({ data, settings }) {
  const { locations, selectedLocation } = data
  const [filter, setFilter] = useState('all')
  const [activeLocation, setActiveLocation] = useState(selectedLocation || null)

  const locationList = Object.values(locations)

  const filteredLocations = locationList.filter(loc => {
    if (filter === 'visited') return loc.visited
    if (filter === 'unvisited') return !loc.visited
    if (filter === 'story') return loc.tags.includes('story-location') || loc.tags.includes('story-critical')
    if (filter === 'combat') return loc.tags.includes('combat-heavy')
    if (filter === 'diplomacy') return loc.tags.includes('diplomacy-heavy')
    return true
  })

  const currentLoc = activeLocation ? locations[activeLocation] : null

  return (
    <div className="locations-panel">
      <div className="panel-header">LOCATION ARCHIVE</div>

      {!currentLoc ? (
        <>
          <div className="location-filters">
            <button 
              className={filter === 'all' ? 'active' : ''} 
              onClick={() => setFilter('all')}
            >
              ALL ({locationList.length})
            </button>
            <button 
              className={filter === 'visited' ? 'active' : ''} 
              onClick={() => setFilter('visited')}
            >
              VISITED ({locationList.filter(l => l.visited).length})
            </button>
            <button 
              className={filter === 'unvisited' ? 'active' : ''} 
              onClick={() => setFilter('unvisited')}
            >
              UNVISITED ({locationList.filter(l => !l.visited).length})
            </button>
            <button 
              className={filter === 'story' ? 'active' : ''} 
              onClick={() => setFilter('story')}
            >
              STORY
            </button>
            <button 
              className={filter === 'combat' ? 'active' : ''} 
              onClick={() => setFilter('combat')}
            >
              COMBAT
            </button>
            <button 
              className={filter === 'diplomacy' ? 'active' : ''} 
              onClick={() => setFilter('diplomacy')}
            >
              DIPLOMACY
            </button>
          </div>

          <div className="location-list">
            {filteredLocations.map(location => (
              <div 
                key={location.id} 
                className={`location-card ${location.visited ? 'visited' : 'unvisited'}`}
                onClick={() => setActiveLocation(location.id)}
              >
                <div className="location-card-header">
                  <h3>{location.name}</h3>
                  <div className="location-tags">
                    {location.tags.slice(0, 3).map((tag, idx) => (
                      <span key={idx} className="tag">{tag}</span>
                    ))}
                  </div>
                </div>
                <p className="location-card-summary">{location.summary}</p>
                {location.visited && (
                  <p className="location-card-visit">First Visit: {location.firstArrival}</p>
                )}
              </div>
            ))}
          </div>
        </>
      ) : (
        <div className="location-detail">
          <button className="back-button" onClick={() => setActiveLocation(null)}>
            ← BACK TO LIST
          </button>

          <h2>{currentLoc.name}</h2>
          <p className="location-summary">{currentLoc.summary}</p>

          <div className="location-meta">
            <div className="meta-item">
              <strong>First Arrival:</strong> {currentLoc.firstArrival}
            </div>
            <div className="meta-item">
              <strong>Tags:</strong> {currentLoc.tags.join(', ')}
            </div>
          </div>

          {currentLoc.events && currentLoc.events.length > 0 && (
            <div className="location-section">
              <h3>Key Events</h3>
              <ul className="event-list">
                {currentLoc.events.map((event, idx) => (
                  <li key={idx}>
                    <strong>{event.title}</strong>
                    <p>{event.description}</p>
                  </li>
                ))}
              </ul>
            </div>
          )}

          {currentLoc.npcs && currentLoc.npcs.length > 0 && (
            <div className="location-section">
              <h3>Notable NPCs</h3>
              <ul className="npc-list">
                {currentLoc.npcs.map((npc, idx) => (
                  <li key={idx}>
                    <strong>{npc.name}:</strong> {npc.note}
                  </li>
                ))}
              </ul>
            </div>
          )}

          {currentLoc.consequences && Object.keys(currentLoc.consequences).length > 0 && (
            <div className="location-section">
              <h3>Consequences</h3>
              {currentLoc.consequences.karma !== 0 && (
                <p>Karma Change: {currentLoc.consequences.karma > 0 ? '+' : ''}{currentLoc.consequences.karma}</p>
              )}
              {currentLoc.consequences.reputation && Object.entries(currentLoc.consequences.reputation).length > 0 && (
                <div>
                  <strong>Reputation Changes:</strong>
                  <ul>
                    {Object.entries(currentLoc.consequences.reputation).map(([faction, change], idx) => (
                      <li key={idx}>{faction}: {change > 0 ? '+' : ''}{change}</li>
                    ))}
                  </ul>
                </div>
              )}
            </div>
          )}
        </div>
      )}
    </div>
  )
}

export default Locations
