import React from 'react'
import '../../styles/panels/Timeline.css'

function Timeline({ data, settings }) {
  const { timeline, timelineMode = 'FULL' } = data
  
  let entries = timeline.entries
  if (timelineMode === 'QUICK') {
    entries = entries.slice(0, 10)
  } else if (timelineMode === 'ARC') {
    entries = entries.filter(e => e.type === 'quest' || e.type === 'milestone')
  }

  const getTypeIcon = (type) => {
    switch (type) {
      case 'quest': return '⚔️'
      case 'location': return '📍'
      case 'combat': return '💥'
      case 'journal': return '📝'
      case 'milestone': return '⭐'
      default: return '•'
    }
  }

  const getTypeClass = (type) => {
    return `timeline-entry-${type}`
  }

  return (
    <div className="timeline-panel">
      <div className="panel-header">TIMELINE - {timelineMode} MODE</div>

      <div className="timeline-mode-selector">
        <span>Mode: </span>
        <span className="mode-info">
          {timelineMode === 'QUICK' && 'Top 10 moments'}
          {timelineMode === 'FULL' && 'Complete chronological history'}
          {timelineMode === 'ARC' && 'Major narrative beats only'}
        </span>
      </div>

      <div className="timeline-container">
        <div className="timeline-line"></div>
        {entries.map((entry, idx) => (
          <div key={entry.id} className={`timeline-entry ${getTypeClass(entry.type)}`}>
            <div className="timeline-marker">{getTypeIcon(entry.type)}</div>
            <div className="timeline-content">
              <div className="timeline-header">
                <span className="timeline-date">{entry.date}</span>
                <span className="timeline-type">[{entry.type.toUpperCase()}]</span>
              </div>
              <h3 className="timeline-title">{entry.title}</h3>
              <p className="timeline-summary">{entry.shortSummary}</p>
              {entry.links && (
                <div className="timeline-links">
                  {entry.links.questId && (
                    <span className="timeline-link">Quest: {entry.links.questId}</span>
                  )}
                  {entry.links.locationId && (
                    <span className="timeline-link">Location: {entry.links.locationId}</span>
                  )}
                  {entry.links.journalId && (
                    <span className="timeline-link">Journal Entry #{entry.links.journalId}</span>
                  )}
                </div>
              )}
            </div>
          </div>
        ))}
      </div>

      <div className="timeline-stats">
        <div>Total Events: {timeline.entries.length}</div>
        <div>Showing: {entries.length}</div>
      </div>
    </div>
  )
}

export default Timeline
