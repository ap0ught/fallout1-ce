import React, { useState, useEffect, useRef } from 'react'
import BootSequence from './BootSequence'
import CommandPrompt from './CommandPrompt'
import VisualPanel from './VisualPanel'
import { COMMANDS } from '../utils/commands'
import '../styles/Terminal.css'

function Terminal({ isBooted }) {
  const [bootComplete, setBootComplete] = useState(false)
  const [history, setHistory] = useState([])
  const [currentPanel, setCurrentPanel] = useState(null)
  const [splitView, setSplitView] = useState(false)
  const [focusMode, setFocusMode] = useState('balanced') // 'balanced', 'terminal', 'visual'
  const [settings, setSettings] = useState({
    crtIntensity: 1,
    scanlines: true,
    textSpeed: 30,
    soundEnabled: false,
    streamMode: false
  })

  const terminalRef = useRef(null)

  useEffect(() => {
    if (isBooted) {
      // Boot sequence will take some time
      const timer = setTimeout(() => {
        setBootComplete(true)
        addToHistory({ 
          type: 'system', 
          text: 'VAULT-TEC UNIFIED OPERATING SYSTEM v2.03.1',
          timestamp: Date.now()
        })
        addToHistory({ 
          type: 'system', 
          text: 'Type HELP for available commands.',
          timestamp: Date.now()
        })
      }, 3000)
      return () => clearTimeout(timer)
    }
  }, [isBooted])

  const addToHistory = (entry) => {
    setHistory(prev => [...prev, entry])
  }

  const handleCommand = (input) => {
    const trimmedInput = input.trim().toUpperCase()
    
    // Add command to history
    addToHistory({ 
      type: 'command', 
      text: `VAULT-TEC> ${input}`,
      timestamp: Date.now()
    })

    // Handle empty input
    if (!trimmedInput) {
      return
    }

    // Parse command
    const [cmd, ...args] = trimmedInput.split(' ')
    const command = COMMANDS[cmd]

    if (!command) {
      addToHistory({ 
        type: 'error', 
        text: `ERROR: Unknown command "${cmd}". Type HELP for available commands.`,
        timestamp: Date.now()
      })
      return
    }

    // Execute command
    const result = command.execute(args, { settings, setSettings })
    
    // Add result to history
    if (result.output) {
      result.output.forEach(line => {
        addToHistory({ 
          type: 'output', 
          text: line,
          timestamp: Date.now()
        })
      })
    }

    // Handle visual panel
    if (result.panel) {
      setCurrentPanel(result.panel)
      setSplitView(true)
    } else if (result.clearPanel) {
      setCurrentPanel(null)
      setSplitView(false)
    }

    // Handle settings changes
    if (result.settings) {
      setSettings(prev => ({ ...prev, ...result.settings }))
    }
  }

  const toggleFocus = (mode) => {
    setFocusMode(mode)
  }

  const crtStyle = {
    '--crt-intensity': settings.crtIntensity,
    '--scanline-opacity': settings.scanlines ? 0.1 : 0
  }

  return (
    <div className={`terminal-container ${settings.streamMode ? 'stream-mode' : ''}`} style={crtStyle}>
      <div className="crt-effect">
        {!bootComplete ? (
          <BootSequence onComplete={() => setBootComplete(true)} />
        ) : (
          <div className={`terminal-layout ${splitView ? 'split-view' : 'full-view'} focus-${focusMode}`}>
            <div className="terminal-section" ref={terminalRef}>
              <div className="terminal-output">
                {history.map((entry, idx) => (
                  <div key={idx} className={`terminal-line ${entry.type}`}>
                    {entry.text}
                  </div>
                ))}
              </div>
              <CommandPrompt onSubmit={handleCommand} />
            </div>
            
            {splitView && currentPanel && (
              <div className="visual-section">
                <div className="focus-controls">
                  <button 
                    onClick={() => toggleFocus('terminal')}
                    className={focusMode === 'terminal' ? 'active' : ''}
                  >
                    FOCUS TERMINAL
                  </button>
                  <button 
                    onClick={() => toggleFocus('balanced')}
                    className={focusMode === 'balanced' ? 'active' : ''}
                  >
                    BALANCED
                  </button>
                  <button 
                    onClick={() => toggleFocus('visual')}
                    className={focusMode === 'visual' ? 'active' : ''}
                  >
                    FOCUS VISUAL
                  </button>
                </div>
                <VisualPanel 
                  type={currentPanel.type} 
                  data={currentPanel.data}
                  settings={settings}
                />
              </div>
            )}
          </div>
        )}
      </div>
      <div className="scanlines"></div>
      <div className="vignette"></div>
    </div>
  )
}

export default Terminal
