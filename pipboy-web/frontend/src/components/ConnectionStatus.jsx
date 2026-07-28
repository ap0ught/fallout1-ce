import React from 'react'

function ConnectionStatus({ connected }) {
  return (
    <div className={`connection-status ${connected ? 'connected' : 'disconnected'}`}>
      <span className="status-indicator"></span>
      <span className="status-text">
        {connected ? 'Connected to Game' : 'Disconnected'}
      </span>
    </div>
  )
}

export default ConnectionStatus
