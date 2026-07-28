import React from 'react'
import '../../styles/panels/Quests.css'

function Quests({ data, settings }) {
  const { quests } = data

  const activeQuests = quests.filter(q => q.status === 'active')
  const completedQuests = quests.filter(q => q.status === 'completed')
  const failedQuests = quests.filter(q => q.status === 'failed')
  const highlights = quests.filter(q => q.highlight)

  const getStatusClass = (status) => {
    return `quest-status-${status}`
  }

  return (
    <div className="quests-panel">
      <div className="panel-header">QUEST LOG</div>

      <div className="quest-summary">
        <div className="summary-stat">
          <span className="stat-label">Active:</span>
          <span className="stat-value">{activeQuests.length}</span>
        </div>
        <div className="summary-stat">
          <span className="stat-label">Completed:</span>
          <span className="stat-value">{completedQuests.length}</span>
        </div>
        <div className="summary-stat">
          <span className="stat-label">Failed:</span>
          <span className="stat-value">{failedQuests.length}</span>
        </div>
      </div>

      {highlights.length > 0 && (
        <div className="quest-section">
          <h3 className="section-title">⭐ HIGHLIGHTS</h3>
          {highlights.map(quest => (
            <div key={quest.id} className={`quest-card highlight ${getStatusClass(quest.status)}`}>
              <div className="quest-header">
                <h4>{quest.name}</h4>
                <span className={`quest-status ${quest.status}`}>{quest.status.toUpperCase()}</span>
              </div>
              <p className="quest-description">{quest.description}</p>
              {quest.outcome && (
                <p className="quest-outcome">
                  <strong>Outcome:</strong> {quest.outcome}
                </p>
              )}
              {quest.linkedLocations && quest.linkedLocations.length > 0 && (
                <div className="quest-locations">
                  <strong>Related Locations:</strong> {quest.linkedLocations.join(', ')}
                </div>
              )}
            </div>
          ))}
        </div>
      )}

      {activeQuests.length > 0 && (
        <div className="quest-section">
          <h3 className="section-title">ACTIVE QUESTS</h3>
          {activeQuests.map(quest => (
            <div key={quest.id} className={`quest-card ${getStatusClass(quest.status)}`}>
              <div className="quest-header">
                <h4>{quest.name}</h4>
                <span className={`quest-status ${quest.status}`}>{quest.status.toUpperCase()}</span>
              </div>
              <p className="quest-description">{quest.description}</p>
            </div>
          ))}
        </div>
      )}

      {completedQuests.length > 0 && (
        <div className="quest-section">
          <h3 className="section-title">COMPLETED QUESTS</h3>
          {completedQuests.map(quest => (
            <div key={quest.id} className={`quest-card ${getStatusClass(quest.status)}`}>
              <div className="quest-header">
                <h4>{quest.name}</h4>
                <span className={`quest-status ${quest.status}`}>{quest.status.toUpperCase()}</span>
              </div>
              <p className="quest-description">{quest.description}</p>
              {quest.outcome && (
                <p className="quest-outcome">
                  <strong>Outcome:</strong> {quest.outcome}
                </p>
              )}
            </div>
          ))}
        </div>
      )}

      {failedQuests.length > 0 && (
        <div className="quest-section">
          <h3 className="section-title">FAILED QUESTS</h3>
          {failedQuests.map(quest => (
            <div key={quest.id} className={`quest-card ${getStatusClass(quest.status)}`}>
              <div className="quest-header">
                <h4>{quest.name}</h4>
                <span className={`quest-status ${quest.status}`}>{quest.status.toUpperCase()}</span>
              </div>
              <p className="quest-description">{quest.description}</p>
            </div>
          ))}
        </div>
      )}
    </div>
  )
}

export default Quests
