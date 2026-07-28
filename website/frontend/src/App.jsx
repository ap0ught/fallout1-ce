import React, { useState, useEffect } from 'react'
import Terminal from './components/Terminal'
import './styles/App.css'

function App() {
  const [isBooted, setIsBooted] = useState(false)

  useEffect(() => {
    // Simulate boot sequence
    const timer = setTimeout(() => {
      setIsBooted(true)
    }, 100)
    return () => clearTimeout(timer)
  }, [])

  return (
    <div className="app">
      <Terminal isBooted={isBooted} />
    </div>
  )
}

export default App
