# Website Backend README

## Data Collector + API Server + Extended Data Generator + Quest Database + Wiki Scraper

The website backend consists of five components:

1. **Data Collector** (`data_collector.py`) - Background service that polls game JSON files
2. **API Server** (`api_server.py`) - REST API that serves data to frontend
3. **Extended Data Generator** (`character_data_generator.py`) - Generates comprehensive character data for terminal UI
4. **Quest Database** (`quest_database.py`) - Maps quest GVARs to wiki names, descriptions, and objectives
5. **Wiki Scraper** (`fallout_wiki_scraper.py`) - Fetches and caches Fallout 1 wiki data with strict lore boundaries

## Installation

```bash
pip install -r requirements.txt
```

## Running

### Start Data Collector (background)
```bash
python data_collector.py --game-dir ../.. --interval 1.0
```

### Start API Server
```bash
python api_server.py --port 5000
```

## API Endpoints

### Base Endpoints (from database)
- `GET /api/current-state` - Current game state with inventory
- `GET /api/stats-history?hours=1` - HP/XP history over time
- `GET /api/skills-current` - Current skill levels
- `GET /api/milestones` - All milestones achieved
- `GET /api/items-collected?limit=100` - Items collected history
- `GET /api/decisions?limit=50` - AI decision history
- `GET /api/session-stats` - Aggregated session statistics
- `GET /api/location-history` - Visited locations
- `GET /api/combat-stats` - Combat statistics
- `GET /api/health` - Health check

### Extended Endpoints (for terminal UI)
- `GET /api/character-extended` - Complete extended character data (all fields below)
- `GET /api/timeline` - Chronological event timeline
- `GET /api/quests` - Quest log with status and outcomes
- `GET /api/locations-extended` - Detailed location dossiers and map data
- `GET /api/journal` - In-character journal entries

### Wiki Endpoints (Fallout 1 lore integration)
- `GET /api/wiki/search?q=<query>` - Search wiki for Fallout 1 content
- `GET /api/wiki/page/<title>` - Fetch specific wiki page data

### Extended Data Features

The extended endpoints support the terminal UI with:
- **Character Identity**: Name, age, pronouns, origin, background
- **Visual Assets**: Portrait and sprite URLs
- **Interactive Map**: Locations with coordinates, visit status, travel routes
- **Timeline**: Chronological "Previously On..." feed (QUICK/FULL/ARC modes)
- **Quest Log**: Active/completed/failed quests with outcomes
- **Location Archive**: Detailed dossiers with events, NPCs, consequences
- **Journal**: In-character diary entries
- **Faction Relations**: Karma and reputation tracking
- **Stream Highlights**: Quick recap bullets for overlays

See [EXTENDED_SCHEMA.md](./EXTENDED_SCHEMA.md) for complete JSON schema documentation.

## Generating Extended Data

The extended data generator creates comprehensive character information for the terminal UI:

```bash
python character_data_generator.py --game-dir ../.. --output ../../character_extended.json
```

This reads base game data (`ai_state.json`, `ai_memory.json`, etc.) and generates:
- Timeline from memory events
- Quest log from quest GVARs + quest database (wiki names/descriptions)
- Location tracking from visited maps
- Journal entries based on progression

The API server automatically generates this data when requested, with 10-second caching.

## Quest Database

The `quest_database.py` module maps quest GVAR values (exported by game) to rich quest information:

**Source:** Fallout Wiki (https://fallout.fandom.com/wiki/Fallout_quests)

**Coverage:** 27 major quests including:
- Main quest line (water chip, destroy vats, defeat master)
- Major side quests (rescue Tandi, Brotherhood initiation, kill deathclaw)
- Critical timers (water supply countdown, vault discovery)

**Features:**
- Quest names from wiki
- Detailed descriptions and objectives
- Status interpretation (GVAR value → active/completed/failed)
- Linked locations
- Rewards and outcomes
- Direct wiki links for full guides

**Usage:**
```python
from quest_database import get_all_quests

quest_gvars = {"GVAR_RESCUE_TANDI": 2, "GVAR_FIND_WATER_CHIP": 1}
quests = get_all_quests(quest_gvars)

# Returns organized quest log with wiki information
# {
#   "active": [...],
#   "completed": [...],
#   "failed": [...],
#   "timers": [...]
# }
```

See `QUEST_DATABASE.md` for complete documentation.

## Database

SQLite database stored at `./database/game_data.db`

Tables:
- `game_states` - Game state snapshots
- `inventory_snapshots` - Inventory over time
- `events` - Game events
- `milestones` - Achievements
- `decisions` - AI decisions
- `skills` - Skill progression
- `stats` - SPECIAL stats
- `session_stats` - Session statistics
- `items_collected` - Item collection history

## Fallout Wiki Integration (NEW)

### Overview

The backend now includes a comprehensive **Fallout Wiki scraper** that fetches and caches data from fallout.fandom.com with strict **Fallout 1 (1997) canon boundaries**.

### Features

- **Automatic caching** (30-day duration)
- **Lore filtering** - Excludes F2/F3/FNV/F4/F76 content
- **Categories**: Locations, quests, characters, factions, items, creatures
- **Smart API usage** - Rate limiting and caching
- **Disk-based storage** - Survives server restarts

### Quick Start

Fetch all Fallout 1 wiki data:
```bash
python fallout_wiki_scraper.py --fetch-all
```

View cache stats:
```bash
python fallout_wiki_scraper.py --stats
```

Search wiki:
```bash
python fallout_wiki_scraper.py --search "water chip"
```

### Wiki API Endpoints

The API server now exposes wiki data:

- `GET /api/wiki/page/<page_title>` - Fetch specific wiki page
- `GET /api/wiki/search?q=<query>` - Search Fallout wiki (F1 only)
- `GET /api/wiki/stats` - Cache statistics

### Lore Policy

**Strict Fallout 1 Canon Only**

✅ **Included:**
- Fallout 1 (1997) content
- Timeline: 2161-2162
- California wasteland locations
- Black Isle/Interplay era lore

❌ **Excluded:**
- Fallout 2, 3, New Vegas, 4, 76 content
- Post-2162 timeline events
- Bethesda/Obsidian retcons
- East Coast locations

See **`LORE_POLICY.md`** for complete policy.

### Usage

```python
from fallout_wiki_scraper import FalloutWikiScraper

scraper = FalloutWikiScraper()

# Fetch page
page = scraper.fetch_page("Vault_13")
print(page['title'])
print(page['extract'])

# Fetch category
locations = scraper.fetch_category_pages("Fallout_locations", limit=50)

# Search
results = scraper.search_wiki("Ian companion")

# Stats
stats = scraper.get_cache_stats()
print(f"Cached: {stats['cached_pages']} pages, {stats['total_size_mb']} MB")
```

### Integration with Quest Database

Quest database is now enhanced with wiki integration:

```python
from quest_database import get_quest_info

quest = get_quest_info("GVAR_RESCUE_TANDI", 2)

# Includes wiki metadata:
# - name: "Rescue Tandi from the Raiders"
# - description: Full wiki description
# - objectives: Step-by-step goals
# - wiki_url: Direct link to guide
```

### Automated Wiki Scraping (GitHub Actions)

A **GitHub Actions workflow** is included to automate wiki scraping:

**Workflow:** `.github/workflows/wiki-scraper.yml`

**Triggers:**
- **Weekly** - Every Monday at 00:00 UTC (keeps cache fresh)
- **Manual** - Via GitHub Actions "Run workflow" button
- **Push** - When scraper code is modified

**What it does:**
1. Fetches all Fallout 1 wiki data
2. Shows cache statistics
3. Uploads cache as artifact (30-day retention)
4. Optionally commits cache to repository

**Manual trigger:**
1. Go to Actions tab in GitHub
2. Select "Fallout Wiki Scraper" workflow
3. Click "Run workflow"
4. Wait ~5 minutes for completion
5. Download cache artifact or check commit

**Local scraping:**
```bash
# Fetch all data
python fallout_wiki_scraper.py --fetch-all

# Force refresh even if cache exists
python fallout_wiki_scraper.py --fetch-all --refresh

# Clear cache and start fresh
python fallout_wiki_scraper.py --clear
python fallout_wiki_scraper.py --fetch-all
```

### Cache Management

**Cache Location:** `website/backend/wiki_cache/`

**Cache Stats:**
- **357 pages** cached (31 locations, 46 quests, 200 characters, 39 factions, 9 items, 29 creatures)
- **~1 MB** total size
- **30-day** expiry (auto-refresh on access)

**Committing Cache:**
- Cache can be committed to repository (recommended for deployment)
- Or excluded via `.gitignore` and fetched via GitHub Action
- Current default: Cache is **included** in repository

### Documentation

- **WIKI_INTEGRATION.md** - Complete wiki scraper guide
- **LORE_POLICY.md** - Canon boundaries and filtering rules
- **QUEST_DATABASE.md** - Quest wiki integration

### Cache Management

Cache location: `wiki_cache/`

Clear cache:
```bash
python fallout_wiki_scraper.py --clear
```

Force refresh:
```bash
python fallout_wiki_scraper.py --fetch-all --refresh
```

### Performance

- **First fetch**: ~3-5 minutes (200+ pages)
- **Cached requests**: <100ms
- **Cache size**: ~10-50 MB
- **Cache expiry**: 30 days

### Dependencies

Added to `requirements.txt`:
- `requests>=2.31.0` (for wiki API calls)
