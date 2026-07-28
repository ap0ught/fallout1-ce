import React from 'react'
import '../../styles/panels/IdCard.css'

function IdCard({ data, settings }) {
  const { character, visuals, stats, relations } = data

  return (
    <div className="id-card-panel">
      <div className="id-card">
        <div className="id-card-header">
          <div className="vault-tec-logo">VAULT-TEC</div>
          <div className="id-card-title">IDENTIFICATION CARD</div>
          <div className="id-serial">SERIAL: VT-{character.origin.replace(/\s/g, '')}-{Date.now().toString().slice(-6)}</div>
        </div>

        <div className="id-card-body">
          <div className="id-photo-section">
            <div className="id-portrait">
              {visuals.portraitUrl ? (
                <img src={visuals.portraitUrl} alt={character.name} />
              ) : (
                <div className="id-portrait-placeholder">
                  [PORTRAIT]
                </div>
              )}
            </div>
            <div className="id-sprite">
              {visuals.spriteUrl ? (
                <img src={visuals.spriteUrl} alt="Character Sprite" />
              ) : (
                <div className="id-sprite-placeholder">
                  [SPRITE]
                </div>
              )}
            </div>
          </div>

          <div className="id-info-section">
            <div className="id-field">
              <span className="id-label">NAME:</span>
              <span className="id-value">{character.name}</span>
            </div>
            <div className="id-field">
              <span className="id-label">ORIGIN:</span>
              <span className="id-value">{character.origin}</span>
            </div>
            <div className="id-field">
              <span className="id-label">BACKGROUND:</span>
              <span className="id-value">{character.background}</span>
            </div>
            <div className="id-field">
              <span className="id-label">AGE:</span>
              <span className="id-value">{character.age} years</span>
            </div>
            <div className="id-field">
              <span className="id-label">PRONOUNS:</span>
              <span className="id-value">{character.pronouns}</span>
            </div>
            <div className="id-field">
              <span className="id-label">LEVEL:</span>
              <span className="id-value">{stats.level}</span>
            </div>
          </div>

          <div className="id-status-section">
            <div className="id-field">
              <span className="id-label">KARMA:</span>
              <span className="id-value karma">{relations.karma}</span>
            </div>
            <div className="id-field">
              <span className="id-label">REPUTATION:</span>
              <div className="reputation-list">
                {relations.factions.slice(0, 3).map((faction, idx) => (
                  <div key={idx} className="reputation-item">
                    {faction.name}: {faction.reputation}
                  </div>
                ))}
              </div>
            </div>
          </div>
        </div>

        <div className="id-card-footer">
          <div className="id-barcode">|||||| || |||| ||| || |||||| || ||| || ||||</div>
          <div className="id-disclaimer">
            PROPERTY OF VAULT-TEC CORPORATION
          </div>
          <div className="id-stamp">AUTHORIZED</div>
        </div>
      </div>
    </div>
  )
}

export default IdCard
