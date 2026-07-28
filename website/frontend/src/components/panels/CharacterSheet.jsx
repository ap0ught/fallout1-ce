import React from 'react'
import '../../styles/panels/CharacterSheet.css'

function CharacterSheet({ data, settings }) {
  const { special, skills, perks, traits, stats } = data

  return (
    <div className="character-sheet-panel">
      <div className="panel-header">CHARACTER SHEET</div>

      <div className="sheet-section">
        <div className="section-title">S.P.E.C.I.A.L. ATTRIBUTES</div>
        <div className="special-cards">
          {Object.entries(special).map(([stat, value]) => (
            <div key={stat} className="special-card">
              <div className="special-card-initial">{stat.charAt(0)}</div>
              <div className="special-card-name">{stat.toUpperCase()}</div>
              <div className="special-card-value">{value}</div>
              <div className="special-card-bar">
                <div 
                  className="special-card-fill" 
                  style={{ width: `${(value / 10) * 100}%` }}
                ></div>
              </div>
            </div>
          ))}
        </div>
      </div>

      <div className="sheet-section">
        <div className="section-title">SKILLS</div>
        <div className="skills-grid">
          {skills.map((skill, idx) => (
            <div key={idx} className={`skill-row ${skill.tag || ''}`}>
              <span className="skill-name">
                {skill.name}
                {skill.tag && <span className="skill-tag">[{skill.tag}]</span>}
              </span>
              <div className="skill-bar-container">
                <div className="skill-bar" style={{ width: `${skill.value}%` }}></div>
              </div>
              <span className="skill-value">{skill.value}%</span>
            </div>
          ))}
        </div>
      </div>

      <div className="sheet-row">
        <div className="sheet-section half">
          <div className="section-title">PERKS</div>
          <div className="perks-list">
            {perks.map((perk, idx) => (
              <div key={idx} className="perk-item">
                <div className="perk-name">{perk.name} {perk.rank > 1 && `(${perk.rank})`}</div>
                <div className="perk-description">{perk.description}</div>
              </div>
            ))}
          </div>
        </div>

        <div className="sheet-section half">
          <div className="section-title">TRAITS</div>
          <div className="traits-list">
            {traits.map((trait, idx) => (
              <div key={idx} className="trait-item">
                <div className="trait-name">{trait.name}</div>
                <div className="trait-description">{trait.description}</div>
              </div>
            ))}
          </div>
        </div>
      </div>

      <div className="sheet-section">
        <div className="section-title">DERIVED STATS</div>
        <div className="derived-stats-grid">
          <div className="derived-stat">
            <span className="derived-label">Hit Points:</span>
            <span className="derived-value">{stats.hp}/{stats.maxHp}</span>
          </div>
          <div className="derived-stat">
            <span className="derived-label">Action Points:</span>
            <span className="derived-value">{stats.ap}/{stats.maxAp}</span>
          </div>
          <div className="derived-stat">
            <span className="derived-label">Armor Class:</span>
            <span className="derived-value">{stats.ac}</span>
          </div>
          <div className="derived-stat">
            <span className="derived-label">Sequence:</span>
            <span className="derived-value">{stats.sequence}</span>
          </div>
          <div className="derived-stat">
            <span className="derived-label">Healing Rate:</span>
            <span className="derived-value">{stats.healingRate}</span>
          </div>
          <div className="derived-stat">
            <span className="derived-label">Critical Chance:</span>
            <span className="derived-value">{stats.criticalChance}%</span>
          </div>
        </div>
      </div>
    </div>
  )
}

export default CharacterSheet
