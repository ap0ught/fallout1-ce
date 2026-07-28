import React from 'react'
import IdCard from './panels/IdCard'
import Dossier from './panels/Dossier'
import CharacterSheet from './panels/CharacterSheet'
import Map from './panels/Map'
import Locations from './panels/Locations'
import Timeline from './panels/Timeline'
import Quests from './panels/Quests'
import Inventory from './panels/Inventory'
import Journal from './panels/Journal'
import Relations from './panels/Relations'
import '../styles/VisualPanel.css'

function VisualPanel({ type, data, settings }) {
  const renderPanel = () => {
    switch (type) {
      case 'id':
        return <IdCard data={data} settings={settings} />
      case 'dossier':
        return <Dossier data={data} settings={settings} />
      case 'sheet':
        return <CharacterSheet data={data} settings={settings} />
      case 'map':
        return <Map data={data} settings={settings} />
      case 'locations':
        return <Locations data={data} settings={settings} />
      case 'timeline':
        return <Timeline data={data} settings={settings} />
      case 'quests':
        return <Quests data={data} settings={settings} />
      case 'inventory':
        return <Inventory data={data} settings={settings} />
      case 'journal':
        return <Journal data={data} settings={settings} />
      case 'relations':
        return <Relations data={data} settings={settings} />
      default:
        return <div>Unknown panel type</div>
    }
  }

  return (
    <div className="visual-panel">
      {renderPanel()}
    </div>
  )
}

export default VisualPanel
