import React, { useState, useEffect } from 'react'

function DataTab() {
  const [quests, setQuests] = useState([])
  const [worldMap, setWorldMap] = useState(null)

  useEffect(() => {
    const fetchQuests = async () => {
      try {
        const response = await fetch('/api/quests')
        if (response.ok) {
          const data = await response.json()
          setQuests(data)
        }
      } catch (error) {
        console.error('Error fetching quests:', error)
      }
    }

    const fetchWorldMap = async () => {
      try {
        const response = await fetch('/api/world-map')
        if (response.ok) {
          const data = await response.json()
          setWorldMap(data)
        }
      } catch (error) {
        console.error('Error fetching world map:', error)
      }
    }

    fetchQuests()
    fetchWorldMap()
  }, [])

  return (
    <div className="data-tab">
      <div className="quests-section">
        <h2>QUEST LOG</h2>
        <div className="quests-list">
          {quests.length === 0 && (
            <div className="no-data">No active quests</div>
          )}
          {quests.map((quest, index) => (
            <div key={index} className={`quest-item ${quest.status}`}>
              <div className="quest-header">
                <span className="quest-name">{quest.name || quest.title}</span>
                <span className={`quest-status ${quest.status}`}>
                  {quest.status?.toUpperCase()}
                </span>
              </div>
              {quest.description && (
                <div className="quest-description">{quest.description}</div>
              )}
              {quest.objective && (
                <div className="quest-objective">→ {quest.objective}</div>
              )}
            </div>
          ))}
        </div>
      </div>

      <div className="map-section">
        <h2>WORLD MAP</h2>
        {worldMap && (
          <div className="map-info">
            <div className="current-location">
              <strong>Current Location:</strong> {worldMap.currentLocation}
            </div>
            <div className="locations-list">
              <h3>Discovered Locations</h3>
              {worldMap.locations && worldMap.locations.map((location, index) => (
                <div key={index} className="location-item">
                  <span className="location-name">{location.name}</span>
                  {location.visited && <span className="visited-badge">✓</span>}
                </div>
              ))}
            </div>
          </div>
        )}
        {!worldMap && (
          <div className="no-data">World map data not available</div>
        )}
      </div>

      <div className="notes-section">
        <h2>NOTES</h2>
        <div className="notes-info">
          <p>Access to holodisks, notes, and other data coming soon...</p>
        </div>
      </div>
    </div>
  )
}

export default DataTab
