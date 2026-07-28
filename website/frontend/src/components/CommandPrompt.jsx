import React, { useState, useRef, useEffect } from 'react'
import '../styles/CommandPrompt.css'

function CommandPrompt({ onSubmit }) {
  const [input, setInput] = useState('')
  const [history, setHistory] = useState([])
  const [historyIndex, setHistoryIndex] = useState(-1)
  const inputRef = useRef(null)

  useEffect(() => {
    // Keep input focused
    if (inputRef.current) {
      inputRef.current.focus()
    }
  }, [])

  const handleSubmit = (e) => {
    e.preventDefault()
    if (input.trim()) {
      setHistory(prev => [...prev, input])
      setHistoryIndex(-1)
      onSubmit(input)
      setInput('')
    }
  }

  const handleKeyDown = (e) => {
    if (e.key === 'ArrowUp') {
      e.preventDefault()
      if (history.length > 0) {
        const newIndex = historyIndex === -1 ? history.length - 1 : Math.max(0, historyIndex - 1)
        setHistoryIndex(newIndex)
        setInput(history[newIndex])
      }
    } else if (e.key === 'ArrowDown') {
      e.preventDefault()
      if (historyIndex >= 0) {
        const newIndex = historyIndex + 1
        if (newIndex >= history.length) {
          setHistoryIndex(-1)
          setInput('')
        } else {
          setHistoryIndex(newIndex)
          setInput(history[newIndex])
        }
      }
    }
  }

  return (
    <form className="command-prompt" onSubmit={handleSubmit}>
      <span className="prompt-text">VAULT-TEC&gt; </span>
      <input
        ref={inputRef}
        type="text"
        value={input}
        onChange={(e) => setInput(e.target.value)}
        onKeyDown={handleKeyDown}
        className="command-input"
        spellCheck={false}
        autoComplete="off"
        autoFocus
      />
      <span className="cursor-blink">_</span>
    </form>
  )
}

export default CommandPrompt
