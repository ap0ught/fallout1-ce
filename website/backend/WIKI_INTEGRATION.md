# Fallout Wiki Integration Guide

## Overview

The Fallout Wiki scraper automatically fetches and caches comprehensive data from the Fallout Wiki (fallout.fandom.com) while maintaining strict **Fallout 1 (1997) canon boundaries**.

## Features

### Data Fetching
- ✅ **Locations**: Towns, vaults, ruins, points of interest
- ✅ **Quests**: Full quest data beyond the 27 GVARs we track
- ✅ **Characters/NPCs**: Dialogue, affiliations, locations
- ✅ **Factions**: Brotherhood, Followers, Hub merchants, etc.
- ✅ **Items**: Weapons, armor, equipment stats
- ✅ **Creatures**: Enemies, encounters, drops

### Caching System
- **30-day cache** - Reduces wiki API load
- **Smart invalidation** - Auto-refreshes stale data
- **Disk-based storage** - Survives server restarts
- **Cache statistics** - Monitor usage and size

### Lore Filtering
- **Fallout 1 only** - Excludes F2, F3, FNV, F4, F76 content
- **Timeline strict** - 2161-2162 only
- **California wasteland** - No East Coast locations
- **Black Isle era** - Original Interplay/Black Isle lore

## Lore Policy

See `LORE_POLICY.md` for comprehensive canon boundaries.

**Key Principles:**
1. Only Fallout 1 (1997) content is canonical
2. Pre-Bethesda timeline integrity maintained
3. No retcons or expansions from later games
4. Conservative approach: when uncertain, exclude

## Installation

```bash
cd website/backend
pip install -r requirements.txt
```

## Usage

### Command Line

**Fetch all Fallout 1 data:**
```bash
python fallout_wiki_scraper.py --fetch-all
```

**Fetch specific page:**
```bash
python fallout_wiki_scraper.py --page "Vault_13"
python fallout_wiki_scraper.py --page "Ian"
```

**Search wiki:**
```bash
python fallout_wiki_scraper.py --search "water chip"
```

**View cache stats:**
```bash
python fallout_wiki_scraper.py --stats
```

**Force refresh cache:**
```bash
python fallout_wiki_scraper.py --fetch-all --refresh
```

**Clear cache:**
```bash
python fallout_wiki_scraper.py --clear
```

### Python API

```python
from fallout_wiki_scraper import FalloutWikiScraper

# Initialize scraper
scraper = FalloutWikiScraper()

# Fetch specific page
page_data = scraper.fetch_page("Vault_13")
print(page_data['title'])
print(page_data['extract'])  # Text extract

# Fetch category
locations = scraper.fetch_category_pages("Fallout_locations", limit=50)
for loc in locations:
    print(f"- {loc['title']}")

# Fetch everything
all_data = scraper.fetch_all_data()
print(f"Locations: {len(all_data['locations'])}")
print(f"Quests: {len(all_data['quests'])}")

# Search
results = scraper.search_wiki("water chip", limit=5)

# Cache stats
stats = scraper.get_cache_stats()
print(f"Cached pages: {stats['cached_pages']}")
print(f"Cache size: {stats['total_size_mb']} MB")
```

### Integration with Character Data Generator

The character data generator automatically enriches quest/location data with wiki information:

```python
from character_data_generator import CharacterDataGenerator

generator = CharacterDataGenerator("../../")
extended_data = generator.generate_extended_data()

# Locations now include wiki descriptions
for loc_id, loc_data in extended_data['locations'].items():
    print(f"{loc_data['name']}: {loc_data['wiki_description']}")

# NPCs include wiki data
for npc in extended_data['npcs']:
    print(f"{npc['name']} ({npc['affiliation']})")
```

## Data Structure

### Page Data Format

```json
{
  "title": "Vault 13",
  "pageid": 12345,
  "extract": "Vault 13 is a fallout shelter located in southern California...",
  "fullurl": "https://fallout.fandom.com/wiki/Vault_13",
  "categories": [
    {"title": "Category:Fallout_locations"},
    {"title": "Category:Vaults"}
  ]
}
```

### Complete Data Export

```json
{
  "locations": [
    {
      "title": "Shady Sands",
      "extract": "A small farming community...",
      "wiki_url": "https://...",
      "categories": ["Fallout_locations"]
    }
  ],
  "quests": [...],
  "characters": [...],
  "factions": [...],
  "items": [...],
  "creatures": [...]
}
```

## Caching Details

### Cache Location
```
website/backend/wiki_cache/
├── index.json              # Cache metadata
├── abc123def456.json      # Cached page data
├── ...
└── fallout1_wiki_data.json  # Complete export
```

### Cache Expiry
- Default: **30 days**
- Configurable via `CACHE_EXPIRY_DAYS`
- Automatically refreshed when expired
- Manual refresh: `--refresh` flag

### Cache Size
- Typical cache: **10-50 MB** for all F1 data
- ~200-500 pages depending on category depth
- Compressed JSON format

## Lore Filtering

### Automatic Filters

**Excluded content markers:**
- "Fallout 2", "Fallout 3", "New Vegas", "Fallout 4", "Fallout 76"
- "Only appears in Fallout 2/3/NV/4"
- "Bethesda", "Obsidian Entertainment"
- References to post-2162 events

**Included content markers:**
- "Fallout 1", "Fallout (1997)"
- "Appears in Fallout"
- "Black Isle", "Interplay"
- Vault 13, Master, Mariposa references

### Manual Curation

Some pages require manual review:

```python
# Check if content is F1 canonical
if not scraper._is_fallout1_content(page_data):
    # Page rejected - contains later game content
    pass
```

### Edge Cases

**Mixed-Game Content:**
- Brotherhood of Steel (appears in all games)
  - **Solution**: Only use F1 section of wiki page
- Marcus (F1 and F2)
  - **Solution**: Only include F1 encounter data

**Retconned Lore:**
- Vault-Tec experiment details differ between games
  - **Solution**: Use only F1 manual/in-game info

## API Rate Limiting

The scraper implements polite API usage:

- **0.5 second delay** between page fetches
- **50 pages per batch** for category fetching
- **User-Agent header** identifies our project
- **Caching** reduces API requests by 99%

## Examples

### Fetch All Location Data

```bash
python fallout_wiki_scraper.py --fetch-all
# Creates: wiki_cache/fallout1_wiki_data.json
```

**Output:**
```
Fetching locations...
  Found 42 locations
Fetching quests...
  Found 27 quests
Fetching characters...
  Found 89 characters
...
```

### Enrich Quest Database

```python
from fallout_wiki_scraper import FalloutWikiScraper
from quest_database import QUEST_DATABASE

scraper = FalloutWikiScraper()

# Fetch quest pages
for gvar_id, quest_info in QUEST_DATABASE.items():
    wiki_name = quest_info['name'].replace(' ', '_')
    page = scraper.fetch_page(wiki_name)
    
    if page:
        # Add wiki description to quest
        quest_info['wiki_description'] = page['extract']
        quest_info['wiki_categories'] = [c['title'] for c in page.get('categories', [])]
```

### Search for NPCs

```python
scraper = FalloutWikiScraper()

# Find all companions
companions = scraper.search_wiki("companion Fallout 1", limit=10)

for comp in companions:
    print(f"{comp['title']}: {comp['extract'][:100]}...")
```

## Troubleshooting

### Issue: "Page not found"

**Cause**: Page doesn't exist or title is incorrect

**Solution**: Check exact wiki page title:
```python
# Correct:
scraper.fetch_page("Vault_13")

# Incorrect:
scraper.fetch_page("Vault 13")  # Needs underscore
```

### Issue: "Filtered out as non-F1 content"

**Cause**: Page contains later game references

**Solution**: Check page manually or adjust filters:
```python
# Bypass filter for specific page (use cautiously)
page_data = scraper.fetch_page("Page_Title", use_cache=False)
# Then manually verify it's F1 content
```

### Issue: "Rate limited by wiki"

**Cause**: Too many requests in short time

**Solution**: Increase delay or use cached data:
```python
time.sleep(1.0)  # Increase to 1 second between requests
```

### Issue: "Cache too large"

**Cause**: Fetched too many pages

**Solution**: Clear cache and limit fetching:
```python
scraper.clear_cache()
pages = scraper.fetch_category_pages("Category", limit=50)  # Limit pages
```

## Performance

### Fetch Times (first run, no cache)
- Single page: **~1 second**
- Category (50 pages): **~30 seconds**
- All data (200+ pages): **~3-5 minutes**

### Fetch Times (with cache)
- Any query: **<100ms** (disk read)

### Recommended Usage
1. Run `--fetch-all` once to populate cache
2. Use cached data for API server
3. Refresh cache weekly or monthly

## Future Enhancements

### Planned Features
1. **Image downloading** - Cache wiki images locally
2. **NPC portraits** - Fetch character artwork
3. **Location maps** - Download map images
4. **Dialogue trees** - Extract full conversation paths
5. **Item icons** - Cache weapon/armor sprites

### Potential Additions
1. **Fallout 2 mode** - Separate lore boundary for F2 streams
2. **Comparison mode** - Show how lore changed across games
3. **Cut content** - Include F1 beta/unused content
4. **Modded content** - Optional support for Fallout Fixt

## Contributing

### Adding New Categories

Edit `DATA_CATEGORIES` in `fallout_wiki_scraper.py`:

```python
DATA_CATEGORIES = {
    "new_category": {
        "category": "Fallout_new_category",
        "fields": ["name", "description", ...]
    }
}
```

### Improving Lore Filters

Update `_is_fallout1_content()` method:

```python
def _is_fallout1_content(self, page_data: Dict[str, Any]) -> bool:
    # Add new filter logic
    if "new_exclude_pattern" in text:
        return False
    return True
```

## References

- **Fallout Wiki API**: https://fallout.fandom.com/api.php
- **MediaWiki API Docs**: https://www.mediawiki.org/wiki/API:Main_page
- **Fallout 1 Wiki Category**: https://fallout.fandom.com/wiki/Category:Fallout
- **Lore Policy**: `LORE_POLICY.md`
- **Quest Database**: `QUEST_DATABASE.md`

## License

Wiki content sourced from Fallout Wiki (CC-BY-SA 3.0).
Scraper code is part of this project's license.

## Support

For issues with the wiki scraper:
1. Check cache stats: `python fallout_wiki_scraper.py --stats`
2. Clear cache and retry: `--clear` then `--fetch-all`
3. Verify wiki page titles match exactly
4. Check network connectivity
5. Review lore filtering in `LORE_POLICY.md`
