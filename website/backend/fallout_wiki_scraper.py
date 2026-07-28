"""
Fallout Wiki Data Scraper and Cache System

Fetches and caches comprehensive Fallout 1 data from the Fallout Wiki,
with strict lore boundaries to maintain Fallout 1 authenticity.

LORE POLICY:
- Only Fallout 1 (1997) content is canonical
- Black Isle/Interplay era lore only
- Excludes retcons and expansions from later games
- Maintains pre-Bethesda timeline integrity
"""

import requests
import json
import os
import time
from typing import Dict, List, Any, Optional
from datetime import datetime, timedelta
import hashlib

# Wiki API configuration
WIKI_API_BASE = "https://fallout.fandom.com/api.php"
CACHE_DIR = os.path.join(os.path.dirname(__file__), "wiki_cache")
CACHE_EXPIRY_DAYS = 30  # Cache for 30 days

# Lore boundary definitions
FALLOUT_1_RELEASE_DATE = "1997-10-10"
CANONICAL_GAMES = ["Fallout", "Fallout 1"]  # Only these are canon for our purposes

# Data categories to fetch
DATA_CATEGORIES = {
    "locations": {
        "category": "Fallout_locations",
        "fields": ["name", "description", "inhabitants", "notable_loot", "quests", "map_marker"]
    },
    "quests": {
        "category": "Fallout_quests",
        "fields": ["name", "description", "objectives", "rewards", "outcomes", "location"]
    },
    "characters": {
        "category": "Fallout_characters",
        "fields": ["name", "description", "affiliation", "location", "quests", "dialogue"]
    },
    "factions": {
        "category": "Fallout_factions",
        "fields": ["name", "description", "headquarters", "leader", "ideology", "reputation"]
    },
    "items": {
        "category": "Fallout_weapons",
        "fields": ["name", "description", "damage", "stats", "location"]
    },
    "creatures": {
        "category": "Fallout_creatures",
        "fields": ["name", "description", "locations", "drops", "combat_info"]
    }
}


class FalloutWikiScraper:
    """
    Scraper for Fallout Wiki with caching and lore filtering
    """
    
    def __init__(self, cache_dir: str = CACHE_DIR):
        self.cache_dir = cache_dir
        self.session = requests.Session()
        self.session.headers.update({
            'User-Agent': 'FalloutTerminalUI/1.0 (Educational/Stream Enhancement)'
        })
        
        # Create cache directory
        os.makedirs(self.cache_dir, exist_ok=True)
        
        # Load cache index
        self.cache_index_path = os.path.join(self.cache_dir, "index.json")
        self.cache_index = self._load_cache_index()
    
    def _load_cache_index(self) -> Dict[str, Any]:
        """Load the cache index or create new one"""
        if os.path.exists(self.cache_index_path):
            with open(self.cache_index_path, 'r', encoding='utf-8') as f:
                return json.load(f)
        return {
            "last_updated": None,
            "categories": {},
            "pages": {}
        }
    
    def _save_cache_index(self):
        """Save the cache index"""
        with open(self.cache_index_path, 'w', encoding='utf-8') as f:
            json.dump(self.cache_index, f, indent=2)
    
    def _get_cache_key(self, identifier: str) -> str:
        """Generate cache key from identifier"""
        return hashlib.md5(identifier.encode()).hexdigest()
    
    def _is_cache_valid(self, cache_key: str) -> bool:
        """Check if cached data is still valid"""
        if cache_key not in self.cache_index['pages']:
            return False
        
        cached_time = datetime.fromisoformat(self.cache_index['pages'][cache_key]['cached_at'])
        expiry_time = cached_time + timedelta(days=CACHE_EXPIRY_DAYS)
        
        return datetime.now() < expiry_time
    
    def _read_cache(self, cache_key: str) -> Optional[Dict[str, Any]]:
        """Read data from cache"""
        cache_file = os.path.join(self.cache_dir, f"{cache_key}.json")
        if os.path.exists(cache_file):
            with open(cache_file, 'r', encoding='utf-8') as f:
                return json.load(f)
        return None
    
    def _write_cache(self, cache_key: str, data: Dict[str, Any], identifier: str):
        """Write data to cache"""
        cache_file = os.path.join(self.cache_dir, f"{cache_key}.json")
        with open(cache_file, 'w', encoding='utf-8') as f:
            json.dump(data, f, indent=2)
        
        # Update index
        self.cache_index['pages'][cache_key] = {
            "identifier": identifier,
            "cached_at": datetime.now().isoformat(),
            "size": len(json.dumps(data))
        }
        self._save_cache_index()
    
    def _is_fallout1_content(self, page_data: Dict[str, Any]) -> bool:
        """
        Determine if wiki page content is Fallout 1 canonical
        
        Filters out:
        - Content from Fallout 2, Tactics, 3, NV, 4, 76
        - Retconned lore from later games
        - Non-Black Isle content
        """
        text = page_data.get('extract', '').lower()
        title = page_data.get('title', '').lower()
        
        # Exclude markers for later games
        exclude_markers = [
            'fallout 2', 'fallout2', 'fo2',
            'fallout 3', 'fallout3', 'fo3',
            'fallout: new vegas', 'new vegas', 'fonv',
            'fallout 4', 'fallout4', 'fo4',
            'fallout 76', 'fallout76', 'fo76',
            'fallout tactics', 'tactics',
            'fallout: brotherhood of steel',
            'only appears in fallout 2',
            'only appears in fallout 3',
            'only appears in new vegas',
            'only appears in fallout 4',
            'bethesda',
            'obsidian entertainment'
        ]
        
        for marker in exclude_markers:
            if marker in text or marker in title:
                return False
        
        # Include markers for Fallout 1
        include_markers = [
            'fallout 1', 'fallout1', 'fallout (1997)',
            'appears in fallout',
            'black isle', 'interplay',
            'vault 13', 'overseer', 'master', 'mariposa'
        ]
        
        # If no include markers found, be conservative
        has_include = any(marker in text or marker in title for marker in include_markers)
        
        # Default to including if uncertain (can be manually filtered later)
        return True  # Will be manually curated in the data
    
    def fetch_page(self, page_title: str, use_cache: bool = True) -> Optional[Dict[str, Any]]:
        """
        Fetch a specific wiki page by title
        
        Args:
            page_title: Wiki page title (e.g., "Vault_13", "Ian")
            use_cache: Whether to use cached data if available
            
        Returns:
            Page data dict or None if not found/not F1 content
        """
        cache_key = self._get_cache_key(page_title)
        
        # Check cache first
        if use_cache and self._is_cache_valid(cache_key):
            return self._read_cache(cache_key)
        
        # Fetch from wiki
        try:
            params = {
                'action': 'query',
                'format': 'json',
                'titles': page_title,
                'prop': 'extracts|categories|info',
                'exintro': True,
                'explaintext': True,
                'inprop': 'url'
            }
            
            response = self.session.get(WIKI_API_BASE, params=params, timeout=10)
            response.raise_for_status()
            data = response.json()
            
            # Extract page data
            pages = data.get('query', {}).get('pages', {})
            if not pages:
                return None
            
            page_data = list(pages.values())[0]
            
            # Check if missing
            if 'missing' in page_data:
                return None
            
            # Filter for Fallout 1 content
            if not self._is_fallout1_content(page_data):
                return None
            
            # Cache and return
            self._write_cache(cache_key, page_data, page_title)
            return page_data
            
        except Exception as e:
            print(f"Error fetching page {page_title}: {e}")
            return None
    
    def fetch_category_pages(self, category: str, limit: int = 100, use_cache: bool = True) -> List[Dict[str, Any]]:
        """
        Fetch all pages in a category
        
        Args:
            category: Category name (e.g., "Fallout_locations")
            limit: Maximum pages to fetch
            use_cache: Whether to use cached data
            
        Returns:
            List of page data dicts
        """
        cache_key = self._get_cache_key(f"category_{category}")
        
        # Check cache
        if use_cache and self._is_cache_valid(cache_key):
            cached = self._read_cache(cache_key)
            if cached:
                return cached.get('pages', [])
        
        # Fetch from wiki
        try:
            pages = []
            continue_token = None
            
            while len(pages) < limit:
                params = {
                    'action': 'query',
                    'format': 'json',
                    'list': 'categorymembers',
                    'cmtitle': f'Category:{category}',
                    'cmlimit': min(50, limit - len(pages)),
                    'cmprop': 'title|ids'
                }
                
                if continue_token:
                    params['cmcontinue'] = continue_token
                
                response = self.session.get(WIKI_API_BASE, params=params, timeout=10)
                response.raise_for_status()
                data = response.json()
                
                members = data.get('query', {}).get('categorymembers', [])
                if not members:
                    break
                
                # Fetch full page data for each member
                for member in members:
                    page_data = self.fetch_page(member['title'], use_cache)
                    if page_data:
                        pages.append(page_data)
                    time.sleep(0.5)  # Rate limiting
                
                # Check for continuation
                if 'continue' in data:
                    continue_token = data['continue'].get('cmcontinue')
                else:
                    break
            
            # Cache results
            cache_data = {
                'category': category,
                'pages': pages,
                'count': len(pages)
            }
            self._write_cache(cache_key, cache_data, f"category_{category}")
            
            return pages
            
        except Exception as e:
            print(f"Error fetching category {category}: {e}")
            return []
    
    def fetch_all_data(self, force_refresh: bool = False) -> Dict[str, Any]:
        """
        Fetch all configured Fallout 1 data from wiki
        
        Args:
            force_refresh: Force refresh even if cache is valid
            
        Returns:
            Complete data dict organized by category
        """
        all_data = {}
        
        for category_name, category_config in DATA_CATEGORIES.items():
            print(f"Fetching {category_name}...")
            pages = self.fetch_category_pages(
                category_config['category'],
                limit=200,
                use_cache=not force_refresh
            )
            all_data[category_name] = pages
            print(f"  Found {len(pages)} {category_name}")
        
        # Update global cache index
        self.cache_index['last_updated'] = datetime.now().isoformat()
        self.cache_index['categories'] = {
            cat: len(pages) for cat, pages in all_data.items()
        }
        self._save_cache_index()
        
        return all_data
    
    def search_wiki(self, query: str, limit: int = 10) -> List[Dict[str, Any]]:
        """
        Search wiki for Fallout 1 content
        
        Args:
            query: Search query
            limit: Maximum results
            
        Returns:
            List of search result dicts
        """
        try:
            params = {
                'action': 'query',
                'format': 'json',
                'list': 'search',
                'srsearch': f'{query} Fallout 1',
                'srlimit': limit
            }
            
            response = self.session.get(WIKI_API_BASE, params=params, timeout=10)
            response.raise_for_status()
            data = response.json()
            
            results = []
            for result in data.get('query', {}).get('search', []):
                page_data = self.fetch_page(result['title'])
                if page_data:
                    results.append(page_data)
            
            return results
            
        except Exception as e:
            print(f"Error searching wiki: {e}")
            return []
    
    def get_cache_stats(self) -> Dict[str, Any]:
        """Get cache statistics"""
        total_size = 0
        file_count = 0
        
        for filename in os.listdir(self.cache_dir):
            if filename.endswith('.json') and filename != 'index.json':
                filepath = os.path.join(self.cache_dir, filename)
                total_size += os.path.getsize(filepath)
                file_count += 1
        
        return {
            "cached_pages": len(self.cache_index.get('pages', {})),
            "cached_categories": self.cache_index.get('categories', {}),
            "total_size_bytes": total_size,
            "total_size_mb": round(total_size / (1024 * 1024), 2),
            "file_count": file_count,
            "last_updated": self.cache_index.get('last_updated'),
            "cache_expiry_days": CACHE_EXPIRY_DAYS
        }
    
    def clear_cache(self):
        """Clear all cached data"""
        for filename in os.listdir(self.cache_dir):
            if filename.endswith('.json'):
                os.remove(os.path.join(self.cache_dir, filename))
        
        self.cache_index = {
            "last_updated": None,
            "categories": {},
            "pages": {}
        }
        self._save_cache_index()


def main():
    """CLI for testing the scraper"""
    import argparse
    
    parser = argparse.ArgumentParser(description='Fallout Wiki Scraper')
    parser.add_argument('--fetch-all', action='store_true', help='Fetch all data')
    parser.add_argument('--refresh', action='store_true', help='Force refresh cache')
    parser.add_argument('--page', type=str, help='Fetch specific page')
    parser.add_argument('--search', type=str, help='Search wiki')
    parser.add_argument('--stats', action='store_true', help='Show cache stats')
    parser.add_argument('--clear', action='store_true', help='Clear cache')
    
    args = parser.parse_args()
    
    scraper = FalloutWikiScraper()
    
    if args.clear:
        scraper.clear_cache()
        print("Cache cleared")
    
    elif args.stats:
        stats = scraper.get_cache_stats()
        print("\nCache Statistics:")
        print(f"  Cached pages: {stats['cached_pages']}")
        print(f"  Total size: {stats['total_size_mb']} MB")
        print(f"  File count: {stats['file_count']}")
        print(f"  Last updated: {stats['last_updated']}")
        print(f"  Cache expiry: {stats['cache_expiry_days']} days")
        print(f"\nCategories:")
        for cat, count in stats['cached_categories'].items():
            print(f"    {cat}: {count} pages")
    
    elif args.page:
        print(f"Fetching page: {args.page}")
        page = scraper.fetch_page(args.page, use_cache=not args.refresh)
        if page:
            print(json.dumps(page, indent=2))
        else:
            print("Page not found or not Fallout 1 content")
    
    elif args.search:
        print(f"Searching for: {args.search}")
        results = scraper.search_wiki(args.search)
        print(f"\nFound {len(results)} results:")
        for result in results:
            print(f"  - {result.get('title')}")
    
    elif args.fetch_all:
        print("Fetching all Fallout 1 data from wiki...")
        data = scraper.fetch_all_data(force_refresh=args.refresh)
        
        output_file = os.path.join(scraper.cache_dir, "fallout1_wiki_data.json")
        with open(output_file, 'w', encoding='utf-8') as f:
            json.dump(data, f, indent=2)
        
        print(f"\nData saved to: {output_file}")
        print("\nSummary:")
        for category, pages in data.items():
            print(f"  {category}: {len(pages)} entries")
    
    else:
        parser.print_help()


if __name__ == '__main__':
    main()
