import React from 'react'
import '../../styles/panels/Journal.css'

function Journal({ data, settings }) {
  const { journal } = data

  return (
    <div className="journal-panel">
      <div className="panel-header">PERSONAL JOURNAL</div>

      <div className="journal-intro">
        <p>In-character reflections and observations from the wasteland.</p>
        <div className="journal-count">Total Entries: {journal.length}</div>
      </div>

      <div className="journal-entries">
        {journal.map((entry, idx) => (
          <div key={idx} className="journal-entry">
            <div className="journal-header">
              <span className="journal-date">{entry.date}</span>
              {entry.tags && entry.tags.length > 0 && (
                <div className="journal-tags">
                  {entry.tags.map((tag, tagIdx) => (
                    <span key={tagIdx} className="journal-tag">{tag}</span>
                  ))}
                </div>
              )}
            </div>
            <div className="journal-content">
              {entry.entry}
            </div>
          </div>
        ))}
      </div>
    </div>
  )
}

export default Journal
