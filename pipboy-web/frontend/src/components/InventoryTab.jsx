import React, { useState, useEffect } from 'react'

function InventoryTab() {
  const [inventory, setInventory] = useState(null)

  useEffect(() => {
    const fetchInventory = async () => {
      try {
        const response = await fetch('/api/inventory')
        if (response.ok) {
          const data = await response.json()
          setInventory(data)
        }
      } catch (error) {
        console.error('Error fetching inventory:', error)
      }
    }

    fetchInventory()
    const interval = setInterval(fetchInventory, 2000)
    return () => clearInterval(interval)
  }, [])

  const handleUseItem = async (itemId) => {
    try {
      await fetch('/api/inventory/use', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ itemId })
      })
    } catch (error) {
      console.error('Error using item:', error)
    }
  }

  const handleEquipItem = async (itemId, slot) => {
    try {
      await fetch('/api/inventory/equip', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ itemId, slot })
      })
    } catch (error) {
      console.error('Error equipping item:', error)
    }
  }

  const handleDropItem = async (itemId, quantity) => {
    try {
      await fetch('/api/inventory/drop', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ itemId, quantity })
      })
    } catch (error) {
      console.error('Error dropping item:', error)
    }
  }

  if (!inventory) {
    return <div className="loading">Loading inventory...</div>
  }

  const weightPercent = (inventory.weight / inventory.maxWeight) * 100

  return (
    <div className="inventory-tab">
      <div className="weight-display">
        <span>Weight: {inventory.weight} / {inventory.maxWeight} lbs</span>
        <div className="weight-bar">
          <div 
            className="weight-fill" 
            style={{ 
              width: `${weightPercent}%`,
              backgroundColor: weightPercent > 90 ? '#ff0000' : '#00ff00'
            }}
          ></div>
        </div>
      </div>

      <div className="equipped-section">
        <h2>EQUIPPED</h2>
        <div className="equipped-grid">
          {inventory.equipped && inventory.equipped.map((item, index) => (
            <div key={index} className="equipped-item">
              <span className="slot-label">{item.slot}:</span>
              <span className="item-name">{item.name}</span>
            </div>
          ))}
        </div>
      </div>

      <div className="items-section">
        <h2>INVENTORY</h2>
        <div className="items-grid">
          {inventory.items && inventory.items.map((item, index) => (
            <div key={index} className="item-card">
              <div className="item-header">
                <span className="item-name">{item.name}</span>
                {item.quantity > 1 && (
                  <span className="item-quantity">x{item.quantity}</span>
                )}
              </div>
              <div className="item-actions">
                <button 
                  className="action-btn"
                  onClick={() => handleUseItem(item.pid || item.id)}
                >
                  Use
                </button>
                {item.equippable && (
                  <button 
                    className="action-btn"
                    onClick={() => handleEquipItem(item.pid || item.id, item.slot)}
                  >
                    Equip
                  </button>
                )}
                <button 
                  className="action-btn drop"
                  onClick={() => handleDropItem(item.pid || item.id, 1)}
                >
                  Drop
                </button>
              </div>
            </div>
          ))}
        </div>
      </div>
    </div>
  )
}

export default InventoryTab
