import React, { useState, useEffect } from 'react'
import StatusTab from './StatusTab'
import StatsTab from './StatsTab'
import InventoryTab from './InventoryTab'
import DataTab from './DataTab'
import io from 'socket.io-client'

function PipBoy() {
  const [activeTab, setActiveTab] = useState('status')
  const [gameData, setGameData] = useState(null)
  const [socket, setSocket] = useState(null)
  const [currentDate, setCurrentDate] = useState(new Date())

  // Update date every second for animation
  useEffect(() => {
    const timer = setInterval(() => {
      setCurrentDate(new Date())
    }, 1000)
    return () => clearInterval(timer)
  }, [])

  useEffect(() => {
    // Connect to WebSocket for real-time updates
    const newSocket = io('http://localhost:5001')
    
    newSocket.on('connect', () => {
      console.log('Connected to Pip-Boy server')
      newSocket.emit('subscribe_updates')
    })

    newSocket.on('game_state_update', (data) => {
      setGameData(data)
    })

    setSocket(newSocket)

    return () => {
      newSocket.close()
    }
  }, [])

  // Also fetch initial data via REST API
  useEffect(() => {
    const fetchData = async () => {
      try {
        const response = await fetch('/api/status')
        if (response.ok) {
          const data = await response.json()
          setGameData(data)
        }
      } catch (error) {
        console.error('Error fetching game data:', error)
      }
    }

    fetchData()
  }, [])

  return (
    <div className="pipboy">
      {/* TV Glass background layer */}
      <div className="tv-glass-background"></div>
      <div className="pipboy-screen">
        <div className="pipboy-sidebar">
          {/* Split-flap date display - moved to sidebar */}
          <div className="split-flap-display split-flap-sidebar">
            <div className="split-flap-digit">{String(currentDate.getDate()).padStart(2, '0')[0]}</div>
            <div className="split-flap-digit">{String(currentDate.getDate()).padStart(2, '0')[1]}</div>
            <div className="split-flap-separator"></div>
            <div className="split-flap-month">{currentDate.toLocaleString('en-US', { month: 'short' }).toUpperCase()}</div>
            <div className="split-flap-separator"></div>
            <div className="split-flap-digit">{String(currentDate.getFullYear())[0]}</div>
            <div className="split-flap-digit">{String(currentDate.getFullYear())[1]}</div>
            <div className="split-flap-digit">{String(currentDate.getFullYear())[2]}</div>
            <div className="split-flap-digit">{String(currentDate.getFullYear())[3]}</div>
            <div className="split-flap-separator"></div>
            <div className="split-flap-time">
              <span className="split-flap-digit">{String(currentDate.getHours()).padStart(2, '0')[0]}</span>
              <span className="split-flap-digit">{String(currentDate.getHours()).padStart(2, '0')[1]}</span>
              <span className="split-flap-colon">:</span>
              <span className="split-flap-digit">{String(currentDate.getMinutes()).padStart(2, '0')[0]}</span>
              <span className="split-flap-digit">{String(currentDate.getMinutes()).padStart(2, '0')[1]}</span>
            </div>
          </div>

          <div className="pipboy-header">
            <img src="/images/logo.webp" alt="Pip-Boy 2000" className="pipboy-logo" />
          </div>
          <div className="pipboy-tabs">
            <button 
              className={activeTab === 'status' ? 'active' : ''} 
              onClick={() => setActiveTab('status')}
            >
              Status
            </button>
            <button 
              className={activeTab === 'stats' ? 'active' : ''} 
              onClick={() => setActiveTab('stats')}
            >
              Special
            </button>
            <button 
              className={activeTab === 'inventory' ? 'active' : ''} 
              onClick={() => setActiveTab('inventory')}
            >
              Inventory
            </button>
            <button 
              className={activeTab === 'data' ? 'active' : ''} 
              onClick={() => setActiveTab('data')}
            >
              Data
            </button>
          </div>
        </div>

        <div className="pipboy-main">
          <div className="pipboy-content">
            {activeTab === 'status' && <StatusTab gameData={gameData} />}
            {activeTab === 'stats' && <StatsTab />}
            {activeTab === 'inventory' && <InventoryTab />}
            {activeTab === 'data' && <DataTab />}
          </div>
        </div>
      </div>
    </div>
  )
}

export default PipBoy
