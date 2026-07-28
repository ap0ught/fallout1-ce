import React, { useState } from 'react'
import '../../styles/panels/Map.css'

function Map({ data, settings }) {
  const { map } = data
  const [selectedLocation, setSelectedLocation] = useState(null)

  const handleLocationClick = (location) => {
    if (location.visited) {
      setSelectedLocation(location)
    }
  }

  return (
    <div className="map-panel">
      <div className="panel-header">WORLD MAP</div>

      <div className="map-container">
        <div className="map-display">
          {/* Simplified map visualization - in production would use actual map image */}
          <div className="map-grid">
            {map.locations.map((location) => (
              <div
                key={location.id}
                className={`map-location ${location.visited ? 'visited' : 'unvisited'} ${location.type}`}
                style={{
                  left: `${location.x}%`,
                  top: `${location.y}%`
                }}
                onClick={() => handleLocationClick(location)}
                title={location.name}
              >
                <div className="location-marker"></div>
                <div className="location-label">{location.name}</div>
              </div>
            ))}

            {/* Draw travel route */}
            <svg className="route-overlay" width="100%" height="100%">
              {map.route.map((routePoint, idx) => {
                if (idx === 0) return null
                const fromLoc = map.locations.find(l => l.id === map.route[idx - 1].locationId)
                const toLoc = map.locations.find(l => l.id === routePoint.locationId)
                if (!fromLoc || !toLoc) return null
                
                return (
                  <line
                    key={idx}
                    x1={`${fromLoc.x}%`}
                    y1={`${fromLoc.y}%`}
                    x2={`${toLoc.x}%`}
                    y2={`${toLoc.y}%`}
                    className="route-line"
                  />
                )
              })}
            </svg>
          </div>
        </div>

        {selectedLocation && (
          <div className="map-location-info">
            <h3>{selectedLocation.name}</h3>
            <p className="location-type">Type: {selectedLocation.type}</p>
            {data.locations[selectedLocation.id] && (
              <>
                <p className="location-summary">
                  {data.locations[selectedLocation.id].summary}
                </p>
                <p className="location-first-visit">
                  First Visit: {data.locations[selectedLocation.id].firstArrival}
                </p>
              </>
            )}
          </div>
        )}
      </div>

      <div className="map-legend">
        <div className="legend-item">
          <span className="legend-marker visited"></span>
          <span>Visited Location</span>
        </div>
        <div className="legend-item">
          <span className="legend-marker unvisited"></span>
          <span>Unvisited Location</span>
        </div>
        <div className="legend-item">
          <span className="legend-line"></span>
          <span>Travel Route</span>
        </div>
      </div>

      <div className="map-stats">
        <div>Total Locations: {map.locations.length}</div>
        <div>Visited: {map.locations.filter(l => l.visited).length}</div>
        <div>Remaining: {map.locations.filter(l => !l.visited).length}</div>
      </div>
    </div>
  )
}

export default Map
