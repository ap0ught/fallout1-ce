import React, { useState, useEffect } from 'react'

function StatsTab() {
  const [stats, setStats] = useState(null)

  useEffect(() => {
    const fetchStats = async () => {
      try {
        const response = await fetch('/api/stats')
        if (response.ok) {
          const data = await response.json()
          setStats(data)
        }
      } catch (error) {
        console.error('Error fetching stats:', error)
      }
    }

    fetchStats()
  }, [])

  if (!stats) {
    return <div className="loading">Loading stats...</div>
  }

  return (
    <div className="stats-tab">
      <div className="special-section">
        <h2>S.P.E.C.I.A.L.</h2>
        <div className="special-grid">
          <div className="stat-item">
            <span className="stat-label">Strength</span>
            <span className="stat-value">{stats.special?.strength || 5}</span>
          </div>
          <div className="stat-item">
            <span className="stat-label">Perception</span>
            <span className="stat-value">{stats.special?.perception || 5}</span>
          </div>
          <div className="stat-item">
            <span className="stat-label">Endurance</span>
            <span className="stat-value">{stats.special?.endurance || 5}</span>
          </div>
          <div className="stat-item">
            <span className="stat-label">Charisma</span>
            <span className="stat-value">{stats.special?.charisma || 5}</span>
          </div>
          <div className="stat-item">
            <span className="stat-label">Intelligence</span>
            <span className="stat-value">{stats.special?.intelligence || 5}</span>
          </div>
          <div className="stat-item">
            <span className="stat-label">Agility</span>
            <span className="stat-value">{stats.special?.agility || 5}</span>
          </div>
          <div className="stat-item">
            <span className="stat-label">Luck</span>
            <span className="stat-value">{stats.special?.luck || 5}</span>
          </div>
        </div>
      </div>

      <div className="skills-section">
        <h2>SKILLS</h2>
        <div className="skills-list">
          {stats.skills && stats.skills.map((skill, index) => (
            <div key={index} className="skill-item">
              <span className="skill-name">{skill.name}</span>
              <span className="skill-value">{skill.value}%</span>
              {skill.tag && <span className="skill-tag">[TAG]</span>}
            </div>
          ))}
        </div>
      </div>

      <div className="perks-section">
        <h2>PERKS</h2>
        <div className="perks-list">
          {stats.perks && stats.perks.map((perk, index) => (
            <div key={index} className="perk-item">
              <span className="perk-name">{perk.name}</span>
              <span className="perk-desc">{perk.description}</span>
            </div>
          ))}
        </div>
      </div>

      <div className="traits-section">
        <h2>TRAITS</h2>
        <div className="traits-list">
          {stats.traits && stats.traits.map((trait, index) => (
            <div key={index} className="trait-item">
              <span className="trait-name">{trait.name || trait}</span>
              <span className="trait-desc">{trait.description || ''}</span>
            </div>
          ))}
        </div>
      </div>
    </div>
  )
}

export default StatsTab
