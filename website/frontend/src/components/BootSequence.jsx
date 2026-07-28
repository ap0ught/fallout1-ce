import React, { useState, useEffect } from 'react'
import '../styles/BootSequence.css'

const BOOT_LINES = [
  'VAULT-TEC UNIFIED OPERATING SYSTEM',
  'VERSION 2.03.1 BUILD 1097',
  '',
  'INITIALIZING SYSTEM MODULES...',
  '> LOADING VAULT-TEC DATABASE.............. [OK]',
  '> INITIALIZING CHARACTER SUBSYSTEMS....... [OK]',
  '> MOUNTING DATA ARCHIVES.................. [OK]',
  '> CONNECTING TO GAME STATE................ [OK]',
  '> LOADING MAP COORDINATES................. [OK]',
  '> SYNCHRONIZING TIMELINE.................. [OK]',
  '',
  'SYSTEM READY.',
  'WELCOME, OVERSEER.',
  ''
]

function BootSequence({ onComplete }) {
  const [lines, setLines] = useState([])
  const [currentIndex, setCurrentIndex] = useState(0)

  useEffect(() => {
    if (currentIndex < BOOT_LINES.length) {
      const timer = setTimeout(() => {
        setLines(prev => [...prev, BOOT_LINES[currentIndex]])
        setCurrentIndex(prev => prev + 1)
      }, currentIndex === 0 ? 500 : (BOOT_LINES[currentIndex] === '' ? 200 : 300))
      return () => clearTimeout(timer)
    } else {
      const finalTimer = setTimeout(() => {
        onComplete()
      }, 500)
      return () => clearTimeout(finalTimer)
    }
  }, [currentIndex, onComplete])

  return (
    <div className="boot-sequence">
      {lines.map((line, idx) => (
        <div key={idx} className={`boot-line ${line.startsWith('>') ? 'progress' : ''}`}>
          {line}
          {idx === lines.length - 1 && !line && <span className="cursor">_</span>}
        </div>
      ))}
    </div>
  )
}

export default BootSequence
