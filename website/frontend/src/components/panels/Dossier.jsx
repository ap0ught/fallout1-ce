import React from 'react'
import '../../styles/panels/Dossier.css'

function Dossier({ data, settings }) {
  const { character, stats, special, skills, currentLocation, streamHighlights } = data

  return (
    <div className="dossier-panel">
      <div className="panel-header">CHARACTER DOSSIER</div>

      <div className="dossier-hero">
        <div className="dossier-sprite">
          {data.visuals.spriteUrl ? (
            <img src={data.visuals.spriteUrl} alt="Character" />
          ) : (
            <div className="sprite-placeholder">[SPRITE]</div>
          )}
        </div>
        <div className="dossier-info">
          <h2>{character.name}</h2>
          <p className="tagline">{character.tagline}</p>
          <div className="quick-stats">
            <div className="stat-badge">Level {stats.level}</div>
            <div className="stat-badge">HP: {stats.hp}/{stats.maxHp}</div>
            <div className="stat-badge">Location: {currentLocation}</div>
          </div>
        </div>
      </div>

      <div className="panel-section">
        <div className="panel-section-title">Stream Highlights</div>
        <ul className="highlight-list">
          {streamHighlights.map((highlight, idx) => (
            <li key={idx}>• {highlight}</li>
          ))}
        </ul>
      </div>

      <div className="panel-section">
        <div className="panel-section-title">S.P.E.C.I.A.L.</div>
        <div className="special-grid">
          {Object.entries(special).map(([stat, value]) => (
            <div key={stat} className="special-stat">
              <div className="special-label">{stat.charAt(0).toUpperCase()}</div>
              <div className="special-value">{value}</div>
              <div className="special-name">{stat}</div>
            </div>
          ))}
        </div>
      </div>

      <div className="panel-section">
        <div className="panel-section-title">Top Skills</div>
        <div className="skills-list">
          {skills.slice(0, 8).map((skill, idx) => (
            <div key={idx} className="skill-item">
              <span className="skill-name">{skill.name}</span>
              <div className="skill-bar-container">
                <div className="skill-bar" style={{ width: `${skill.value}%` }}></div>
                <span className="skill-value">{skill.value}%</span>
              </div>
            </div>
          ))}
        </div>
      </div>
    </div>
  )
}

export default Dossier
