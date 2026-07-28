import React, { useState, useEffect } from 'react'

function StatusTab({ gameData }) {
  const [status, setStatus] = useState(null)

  useEffect(() => {
    const fetchStatus = async () => {
      try {
        const response = await fetch('/api/status')
        if (response.ok) {
          const data = await response.json()
          setStatus(data)
        }
      } catch (error) {
        console.error('Error fetching status:', error)
      }
    }

    fetchStatus()
    const interval = setInterval(fetchStatus, 1000)
    return () => clearInterval(interval)
  }, [])

  if (!status) {
    return <div className="loading">Loading status...</div>
  }

  const hpPercent = (status.hp / status.maxHp) * 100
  const apPercent = (status.ap / status.maxAp) * 100

  return (
    <div className="status-tab">
      <div className="status-section">
        <h2>CURRENT STATUS</h2>
        
        <div className="status-bar">
          <label>Hit Points</label>
          <div className="bar-container">
            <div className="bar hp-bar" style={{ width: `${hpPercent}%` }}></div>
          </div>
          <span className="bar-value">{status.hp} / {status.maxHp}</span>
        </div>

        <div className="status-bar">
          <label>Action Points</label>
          <div className="bar-container">
            <div className="bar ap-bar" style={{ width: `${apPercent}%` }}></div>
          </div>
          <span className="bar-value">{status.ap} / {status.maxAp}</span>
        </div>

        <div className="status-info">
          <div className="info-row">
            <span>Level:</span>
            <span>{status.level}</span>
          </div>
          <div className="info-row">
            <span>Experience:</span>
            <span>{status.experience}</span>
          </div>
          <div className="info-row">
            <span>Location:</span>
            <span>{status.location}</span>
          </div>
          <div className="info-row">
            <span>Combat Status:</span>
            <span className={status.inCombat ? 'combat-active' : 'combat-inactive'}>
              {status.inCombat ? 'IN COMBAT' : 'NORMAL'}
            </span>
          </div>
        </div>

        {status.radiation > 0 && (
          <div className="warning">
            ⚠ RADIATION: {status.radiation} rads
          </div>
        )}

        {status.poisoned && (
          <div className="warning">
            ⚠ POISONED
          </div>
        )}
      </div>

      <div className="actions-section">
        <h3>QUICK ACTIONS</h3>
        <button onClick={handleRest}>Rest (1 hour)</button>
      </div>
    </div>
  )
}

async function handleRest() {
  try {
    await fetch('/api/rest', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ hours: 1 })
    })
  } catch (error) {
    console.error('Error resting:', error)
  }
}

export default StatusTab
