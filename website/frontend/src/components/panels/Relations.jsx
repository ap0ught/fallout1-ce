import React from 'react'
import '../../styles/panels/Relations.css'

function Relations({ data, settings }) {
  const { relations } = data

  const getReputationClass = (reputation) => {
    const rep = reputation.toLowerCase()
    if (rep.includes('idolized')) return 'reputation-idolized'
    if (rep.includes('liked') || rep.includes('friend')) return 'reputation-liked'
    if (rep.includes('neutral')) return 'reputation-neutral'
    if (rep.includes('disliked') || rep.includes('antipathy')) return 'reputation-disliked'
    if (rep.includes('hated') || rep.includes('vilified')) return 'reputation-hated'
    return 'reputation-unknown'
  }

  const getReputationBar = (standing) => {
    // Standing ranges from -100 to +100, normalize to 0-100 for display
    const normalized = (standing + 100) / 2
    return normalized
  }

  return (
    <div className="relations-panel">
      <div className="panel-header">FACTION RELATIONS</div>

      <div className="karma-section">
        <h3>Overall Karma</h3>
        <div className="karma-display">{relations.karma}</div>
      </div>

      <div className="factions-section">
        <h3 className="section-title">FACTION STANDINGS</h3>
        <div className="factions-list">
          {relations.factions.map((faction, idx) => (
            <div key={idx} className="faction-item">
              <div className="faction-header">
                <span className="faction-name">{faction.name}</span>
                <span className={`faction-reputation ${getReputationClass(faction.reputation)}`}>
                  {faction.reputation}
                </span>
              </div>
              <div className="faction-bar-container">
                <div 
                  className={`faction-bar ${getReputationClass(faction.reputation)}`}
                  style={{ width: `${getReputationBar(faction.standing)}%` }}
                ></div>
              </div>
              <div className="faction-standing">Standing: {faction.standing}</div>
            </div>
          ))}
        </div>
      </div>

      <div className="relations-legend">
        <h4>Reputation Levels</h4>
        <div className="legend-items">
          <div className="legend-item">
            <span className="legend-color reputation-idolized"></span>
            <span>Idolized (75+)</span>
          </div>
          <div className="legend-item">
            <span className="legend-color reputation-liked"></span>
            <span>Liked (25-74)</span>
          </div>
          <div className="legend-item">
            <span className="legend-color reputation-neutral"></span>
            <span>Neutral (-24 to 24)</span>
          </div>
          <div className="legend-item">
            <span className="legend-color reputation-disliked"></span>
            <span>Disliked (-25 to -74)</span>
          </div>
          <div className="legend-item">
            <span className="legend-color reputation-hated"></span>
            <span>Hated (-75 or less)</span>
          </div>
        </div>
      </div>
    </div>
  )
}

export default Relations
