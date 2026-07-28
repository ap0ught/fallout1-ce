# Fallout 1 Lore Policy

## Canon Boundaries for Stream Companion UI

This document defines what constitutes "canonical" lore for the Fallout 1 stream companion website to maintain authenticity and avoid anachronisms.

## Core Principle

**Only Fallout 1 (1997) content is canonical for this project.**

The stream companion is designed to enhance viewing of an AI playing **Fallout 1 specifically**, not the broader Fallout universe. Therefore, we must respect the game's original lore boundaries.

## What is Canonical

### ✅ Included (Fallout 1 Canon)

1. **Fallout 1 (1997) - Black Isle Studios / Interplay**
   - All in-game content, dialogue, locations, characters
   - The Vault Dweller's story
   - Original ending slides
   - Fallout 1 Manual lore
   - Original Fallout Bible entries (by Chris Avellone) that specifically reference F1

2. **Pre-War Backstory Established in F1**
   - Great War (October 23, 2077)
   - Vault-Tec experiments as revealed in F1
   - Master's origin story
   - FEV history (as told in F1)
   - Vault 13 history

3. **Wasteland Circa 2161**
   - California wasteland locations only
   - Factions as they exist in 2161:
     - Brotherhood of Steel (original chapter)
     - Followers of the Apocalypse
     - Hub merchants
     - Shady Sands
     - Junktown factions
     - Cathedral/Unity
   - Super Mutants (first generation, Master's army)
   - Ghouls (as established in Necropolis)

4. **Technology & Items**
   - Weapons/armor/items available in F1
   - Technology level consistent with F1 (e.g., no plasma rifles everywhere)
   - Pip-Boy 2000 interface
   - Original S.P.E.C.I.A.L. system

## What is NOT Canonical

### ❌ Excluded (Post-F1 Content)

1. **Fallout 2 (1998) and Later Games**
   - Chosen One's story
   - NCR formation and expansion
   - Enclave (not in F1)
   - New Reno, San Francisco, etc.
   - Arroyo events
   - Any content introduced in F2

2. **Fallout 3, New Vegas, 4, 76**
   - East Coast lore
   - Capital Wasteland
   - Mojave Wasteland
   - The Commonwealth
   - Appalachia
   - Brotherhood of Steel changes (Eastern chapter, etc.)
   - Any Bethesda/Obsidian retcons

3. **Fallout Tactics & Brotherhood of Steel**
   - Midwest Brotherhood
   - Any non-canon game content

4. **Later Retcons & Expansions**
   - Lore contradictions with F1
   - Post-2161 timeline events
   - Technology that didn't exist in F1
   - Faction changes post-F1

## Specific Lore Boundaries

### Factions

**Brotherhood of Steel**
- ✅ Original Lost Hills bunker chapter
- ✅ High Elder Maxson
- ✅ Initiation quest (retrieving holodisk from The Glow)
- ❌ Eastern Brotherhood (F3)
- ❌ Mojave chapter (FNV)
- ❌ Any post-2161 Brotherhood history

**Followers of the Apocalypse**
- ✅ Boneyard library chapter
- ✅ Nicole and original members
- ✅ Humanitarian mission as established in F1
- ❌ Mojave presence (FNV)
- ❌ Post-2161 expansion

**Hub Merchants**
- ✅ Water Merchants
- ✅ Far Go Traders
- ✅ Crimson Caravan (original)
- ❌ Later caravan companies from F2/FNV

### Locations

**Only California Wasteland Locations from F1:**
- Vault 13
- Vault 15
- Shady Sands
- Junktown
- The Hub
- Necropolis
- Brotherhood of Steel bunker
- Mariposa Military Base
- The Glow
- Boneyard (Los Angeles ruins)
- Cathedral
- Raider camps
- Random encounter locations

**Excluded:**
- Any locations introduced in later games
- East/Midwest/Mountain regions
- Post-2161 settlements

### Technology

**Fallout 1 Tech Level:**
- ✅ Laser/plasma weapons (rare, advanced tech)
- ✅ Power Armor (Brotherhood exclusive, rare)
- ✅ Pre-war robots and computers
- ✅ Pip-Boy 2000 (not 3000)
- ❌ Advanced tech from later games
- ❌ Enclave tech (not in F1)
- ❌ Weapons/armor from F2+

### Timeline

**Strict Date: 2161-2162**
- Game takes place December 5, 2161 onward
- 84 years after the Great War
- 10 years before Fallout 2
- Any events post-2162 are not canon for our purposes

## Implementation Guidelines

### Wiki Scraping

When scraping the Fallout Wiki:

1. **Filter by Game Origin**
   - Only accept pages tagged "Fallout 1" or "Fallout (1997)"
   - Reject pages marked "Fallout 2", "Fallout 3", etc.
   - Be cautious with "Appears in Fallout, Fallout 2..." - use only F1 info

2. **Content Validation**
   - Check page text for later game references
   - Exclude sections about "Later appearances"
   - Use only the "Fallout 1" tab/section if available

3. **Automatic Filters**
   - Exclude pages mentioning: "Enclave", "NCR Republic", "Chosen One", "Lone Wanderer", "Courier", "Sole Survivor"
   - Exclude dates after 2162
   - Exclude locations outside California

### Quest Database

- Only 27 quests from Fallout 1
- Original quest outcomes only
- No F2/FNV quest references

### Character/NPC Data

**Include:**
- Ian, Katja, Tycho, Dogmeat (F1 companions)
- Vault Dweller
- Master/Lieutenant
- Town leaders (Aradesh, Killian, etc.)

**Exclude:**
- Chosen One and F2+ companions
- Marcus (appears in both but use only F1 info)
- Any F2+ characters

### Dialogue & Flavor Text

- Use only F1 dialogue trees
- Vault-Tec corporate humor from F1
- Original ending narration style
- No "war never changes" meta-references from later games

## Handling Ambiguity

When uncertain if content is F1 canon:

1. **Check Release Date**: If referenced content was added after 1997, exclude it
2. **Check Source**: If it's from F1 manual/game files, include it
3. **Conservative Approach**: When in doubt, exclude it
4. **Manual Curation**: Flag for human review

## Why These Boundaries Matter

1. **Authenticity**: Viewers are watching F1, not F2 or F3
2. **Immersion**: Anachronisms break the 2161 wasteland atmosphere
3. **Educational**: Teaches players about original Fallout lore
4. **Respectful**: Honors the original game design

## Acceptable "Expanded Universe"

The following are acceptable even though technically post-F1:

1. **Fallout Bible** (Chris Avellone's lore document)
   - Only sections that clarify F1 content
   - Exclude F2-specific clarifications

2. **Developer Interviews** about F1
   - Intent/design philosophy
   - Cut content explanations

3. **Technical Documentation**
   - GVAR meanings
   - Quest mechanics
   - Engine details

## Examples

### ✅ Good: Canonical Fallout 1 Data

```json
{
  "name": "Ian",
  "description": "A scavenger from Shady Sands with skill in small guns.",
  "location": "Shady Sands",
  "faction": "Independent",
  "appears_in": "Fallout (1997)",
  "wiki_url": "https://fallout.fandom.com/wiki/Ian_(Fallout)"
}
```

### ❌ Bad: Post-F1 Content

```json
{
  "name": "NCR",
  "description": "The New California Republic, formed in 2189...",
  "appears_in": "Fallout 2, Fallout: New Vegas",
  "wiki_url": "..."
}
```

### ⚠️ Needs Filtering: Mixed Content

```json
{
  "name": "Brotherhood of Steel",
  "description": "A techno-religious organization...",
  "appears_in": "Fallout, Fallout 2, Fallout 3, Fallout 4, Fallout 76",
  "note": "USE ONLY FALLOUT 1 SECTION"
}
```

## Future Considerations

If the project expands to cover Fallout 2:

1. Create separate lore boundary document for F2
2. Add game selection toggle in UI
3. Maintain separate data caches per game
4. Never mix F1 and F2 data in single view

## References

- **Fallout 1 Manual**: Original lore bible
- **Fallout Wiki**: https://fallout.fandom.com/wiki/Fallout (F1 specific)
- **The Vault (archive)**: https://fallout.fandom.com/wiki/The_Vault
- **Fallout Bible**: Chris Avellone's lore document (use F1 sections only)
- **No Mutants Allowed**: Community lore discussions (verify against F1 game)

## Maintainer Notes

When adding new wiki data:

1. Verify game of origin
2. Check timeline dates (must be pre-2162)
3. Confirm no F2+ references
4. Flag anything uncertain for review
5. Document any manual curation decisions

**Bottom Line**: If it didn't exist or happen in Fallout 1 (1997), it doesn't belong in this stream companion.
