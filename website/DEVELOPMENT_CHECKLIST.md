# Development Checklist - Stream Companion Website

## Purpose
This checklist ensures the Fallout 1 stream companion website is optimized for:
1. **Stream Readability** - Viewers can easily follow on Twitch
2. **"Previously On..." Usefulness** - Catching up on story is intuitive
3. **Terminal Authenticity** - Fallout/Vault-Tec aesthetic is immersive

## Stream Readability

### Visual Clarity
- [ ] **Font Size**: Minimum 16px for terminal text (20px+ in Stream Mode)
- [ ] **Line Spacing**: At least 1.5 line height for readability
- [ ] **Contrast**: Green text on black background passes WCAG AA
- [ ] **Glow Effects**: Subtle enough to not cause eye strain
- [ ] **Scanlines**: Opacity adjustable (on/off toggle works)

### Text Content
- [ ] **Concise Output**: Terminal responses max 10-15 lines
- [ ] **Stream Highlights**: Top 5 bullets fit on one screen
- [ ] **Location Summaries**: 5-10 lines max per location
- [ ] **Quest Descriptions**: 1-2 sentences, clear outcomes
- [ ] **Journal Entries**: 3-5 sentences per entry

### Layout
- [ ] **Split View Balance**: Terminal and visual panel both readable at 1920x1080
- [ ] **Focus Modes**: FOCUS VISUAL makes panels readable from distance
- [ ] **Stream Mode**: Increases font 20%, reduces visual clutter
- [ ] **No Microtext**: All text legible on 1080p stream
- [ ] **Overflow Handling**: Long text scrolls or truncates gracefully

### Testing Checklist
- [ ] View at 1920x1080 resolution (standard stream res)
- [ ] View at 1280x720 resolution (reduced bandwidth)
- [ ] Test with STREAM MODE ON
- [ ] Test FOCUS VISUAL mode for panels
- [ ] Check readability from 6 feet away (viewer distance)
- [ ] Screenshot in OBS - verify all text is legible

## "Previously On..." Usefulness

### Timeline Feature
- [ ] **QUICK Mode**: Shows top 10 most important events
- [ ] **FULL Mode**: Complete chronological history
- [ ] **ARC Mode**: Only major story beats
- [ ] **Event Types**: Quest, Location, Combat, Journal clearly distinguished
- [ ] **Links**: Events link to related quests/locations/journal
- [ ] **Dates**: Relative time (Day 45) or session markers

### Recap Navigation
- [ ] **DOSSIER**: One-screen character summary works
- [ ] **Stream Highlights**: Answers "what's happening right now?"
- [ ] **MAP**: Shows where character has been and current location
- [ ] **LOCATIONS**: Each location has "what happened here?"
- [ ] **QUESTS**: Clear active vs. completed vs. failed
- [ ] **JOURNAL**: In-character perspective on recent events

### Catchup Flow Test
Test that new viewers can understand the story:
1. [ ] Type `HELP` - see available commands
2. [ ] Type `DOSSIER` - get character overview
3. [ ] Type `TIMELINE QUICK` - see top 10 moments
4. [ ] Type `MAP` - understand journey so far
5. [ ] Type `QUESTS` - know current objectives
6. [ ] **Under 2 minutes** - viewer is caught up

### Content Quality
- [ ] **Context**: Every event has enough context to understand
- [ ] **Connections**: Links between quests/locations/journal work
- [ ] **Outcomes**: Completed quests show what happened
- [ ] **Consequences**: Karma/reputation changes are visible
- [ ] **NPCs**: Notable NPCs are mentioned in location dossiers

## Terminal Authenticity

### Vault-Tec Aesthetic
- [ ] **Boot Sequence**: Vault-Tec branding, system initialization
- [ ] **Prompt**: "VAULT-TEC>" looks official
- [ ] **Headers**: Bureaucratic labels (SERIAL, AUTHORIZED, etc.)
- [ ] **ID Card**: Vault-Tec design elements (barcode, stamps)
- [ ] **Error Messages**: "ERROR:" prefix, system-style responses

### CRT Effects
- [ ] **Scanlines**: Present and adjustable
- [ ] **Glow**: Text has phosphor glow effect
- [ ] **Vignette**: Corners darkened appropriately
- [ ] **Curvature**: SVG clip-path applies CRT shape
- [ ] **Flicker**: Subtle animation (not distracting)
- [ ] **Color**: Green/amber terminal colors authentic

### Command System
- [ ] **Keyboard-First**: All navigation works via typing
- [ ] **Command History**: Up/down arrow recalls commands
- [ ] **Case Insensitive**: HELP = help = Help
- [ ] **Unknown Commands**: Helpful error messages
- [ ] **Auto-Complete**: Consider suggesting commands (future)

### Panel Design
- [ ] **ID Card**: Looks like official Vault-Tec document
- [ ] **Character Sheet**: Fallout 1 stat layout
- [ ] **Map**: Retro map style, not modern UI
- [ ] **Timeline**: Computer log aesthetic
- [ ] **Borders**: All panels have terminal-style borders

## Technical Quality

### Performance
- [ ] **Fast Boot**: Boot sequence completes in 3-5 seconds
- [ ] **Responsive Commands**: Terminal responds immediately
- [ ] **No Lag**: Split view doesn't cause slowdown
- [ ] **API Latency**: Data loads within 1 second
- [ ] **Build Size**: Under 300KB gzipped

### Cross-Browser
- [ ] **Chrome**: All features work
- [ ] **Firefox**: All features work
- [ ] **Edge**: All features work
- [ ] **Safari**: All features work (clip-path supported)

### Accessibility
- [ ] **Keyboard Navigation**: Full keyboard control
- [ ] **Screen Readers**: Consider ARIA labels (optional)
- [ ] **Focus Indicators**: Visible focus states
- [ ] **Contrast**: Passes WCAG AA
- [ ] **Settings**: Scanlines/glow can be disabled

### Data Handling
- [ ] **Graceful Fallback**: Works with minimal data
- [ ] **Error Messages**: Helpful when API fails
- [ ] **Missing Fields**: Shows "Unknown" not crashes
- [ ] **Empty States**: "No quests yet" instead of blank
- [ ] **Mock Data**: Development works without backend

## Integration Testing

### With Backend API
- [ ] **API Connection**: Frontend connects to localhost:5000
- [ ] **Extended Data**: `/api/character-extended` returns valid JSON
- [ ] **Timeline**: `/api/timeline` works
- [ ] **Quests**: `/api/quests` works
- [ ] **Locations**: `/api/locations-extended` works
- [ ] **Error Handling**: Friendly message if API down

### With Game Data
- [ ] **ai_state.json**: Reads HP, skills, location
- [ ] **ai_memory.json**: Parses for timeline events
- [ ] **character_data.json**: Loads character info (if exists)
- [ ] **Extended Generator**: Produces valid JSON
- [ ] **Cache**: 10-second cache works

### OBS Integration
- [ ] **Browser Source**: Works as OBS browser source
- [ ] **Transparency**: Background transparent (optional)
- [ ] **Resolution**: Fits 1920x1080, 1280x720
- [ ] **Refresh**: Auto-refresh setting works
- [ ] **Performance**: No dropped frames in OBS

## Pre-Launch Checklist

### Documentation
- [ ] Frontend README complete
- [ ] Backend README updated
- [ ] EXTENDED_SCHEMA.md accurate
- [ ] Command reference in HELP is complete
- [ ] Setup instructions tested

### Code Quality
- [ ] No console errors
- [ ] No lint errors
- [ ] Build succeeds
- [ ] All imports resolve
- [ ] No dead code

### Final Testing
- [ ] Run `npm run build` succeeds
- [ ] Run `npm run dev` works
- [ ] Backend API starts without errors
- [ ] Extended data generator runs
- [ ] All commands work in terminal
- [ ] All panels display correctly
- [ ] Settings persist (if applicable)

## Stream Readability Score

Test on actual stream setup:
- [ ] **Viewer Feedback**: Can 3+ viewers read text easily?
- [ ] **Distance Test**: Readable from 6 feet away?
- [ ] **Quick Glance**: Story status clear in 5 seconds?
- [ ] **30-Second Recap**: New viewer can understand in 30s?

## Success Criteria

✅ **Stream Ready** if:
1. All text readable on 1080p stream
2. New viewers can catch up in under 2 minutes
3. CRT aesthetic is immersive and authentic
4. Terminal commands are intuitive
5. No technical issues in OBS

✅ **Launch Ready** if:
1. All checklist items completed
2. Documentation complete
3. Sample data works
4. Backend API stable
5. Build deploys successfully

## Notes

- Priority: Stream readability > Feature completeness
- Terminal aesthetic > Modern UI
- Simple and clear > Complex and fancy
- Fallout lore accuracy matters for immersion

## Feedback Loop

After stream testing:
1. Collect viewer feedback on readability
2. Note which commands are most used
3. Identify confusing UI elements
4. Measure catchup time for new viewers
5. Iterate based on data
