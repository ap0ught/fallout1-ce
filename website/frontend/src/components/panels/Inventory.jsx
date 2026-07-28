import React from 'react'
import '../../styles/panels/Inventory.css'

function Inventory({ data, settings }) {
  const { inventory } = data

  return (
    <div className="inventory-panel">
      <div className="panel-header">INVENTORY</div>

      <div className="inventory-section">
        <h3 className="section-title">EQUIPPED</h3>
        <div className="equipped-grid">
          {inventory.equipped.map((item, idx) => (
            <div key={idx} className="equipped-item">
              <div className="item-slot">{item.slot}</div>
              <div className="item-name">{item.name}</div>
              <div className="item-pid">PID: {item.pid}</div>
            </div>
          ))}
        </div>
      </div>

      <div className="inventory-section">
        <h3 className="section-title">NOTABLE ITEMS</h3>
        <div className="notable-items-list">
          {inventory.notable.map((item, idx) => (
            <div key={idx} className="notable-item">
              <div className="item-header">
                <span className="item-name">{item.name}</span>
                <span className="item-quantity">x{item.quantity}</span>
              </div>
              {item.note && (
                <div className="item-note">{item.note}</div>
              )}
              <div className="item-pid">PID: {item.pid}</div>
            </div>
          ))}
        </div>
      </div>

      <div className="inventory-footer">
        <div className="inventory-stat">
          <span className="stat-label">Total Equipped:</span>
          <span className="stat-value">{inventory.equipped.length}</span>
        </div>
        <div className="inventory-stat">
          <span className="stat-label">Notable Items:</span>
          <span className="stat-value">{inventory.notable.length}</span>
        </div>
      </div>
    </div>
  )
}

export default Inventory
