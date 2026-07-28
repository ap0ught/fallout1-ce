import React, { useState, useEffect } from 'react'
import PipBoy from './components/PipBoy'
import ProfileBrowser from './components/ProfileBrowser'
import ConnectionStatus from './components/ConnectionStatus'

function App() {
  const [view, setView] = useState('pipboy') // 'pipboy' or 'profiles'
  const [connected, setConnected] = useState(false)

  useEffect(() => {
    // Check backend connection
    const checkConnection = async () => {
      try {
        const response = await fetch('/api/health')
        const data = await response.json()
        setConnected(data.status === 'ok')
      } catch (error) {
        setConnected(false)
      }
    }

    checkConnection()
    const interval = setInterval(checkConnection, 5000)
    return () => clearInterval(interval)
  }, [])

  return (
    <div className="app">
      <ConnectionStatus connected={connected} />
      
      <div className="main-menu">
        <button 
          className={view === 'pipboy' ? 'active' : ''} 
          onClick={() => setView('pipboy')}
        >
          PIP-BOY
        </button>
        <button 
          className={view === 'profiles' ? 'active' : ''} 
          onClick={() => setView('profiles')}
        >
          PROFILES
        </button>
      </div>

      {view === 'pipboy' && <PipBoy />}
      {view === 'profiles' && <ProfileBrowser />}
    </div>
  )
}

export default App
