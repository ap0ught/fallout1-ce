"""
Strategy Context Module (OPTIONAL TOGGLE)

Provides gameplay optimization:
- Optimal Paths (best routes through areas)
- Warnings (hidden dangers, tough enemies)
- Build Advice (skill recommendations, equipment suggestions)

Can be disabled for harder/more immersive gameplay
"""

import json
import os
from typing import Dict, List, Optional
from dataclasses import dataclass


@dataclass
class StrategyAdvice:
    """Strategy recommendations"""
    optimal_path: str
    warnings: List[str]
    build_advice: List[str]
    tactical_suggestions: List[str]


class StrategyContextProvider:
    """Provides optimization hints (can be toggled off)"""
    
    def __init__(self, data_dir: str = "agent/data", enabled: bool = False):
        self.data_dir = data_dir
        self.enabled = enabled
        self.strategies = self._load_strategies()
    
    def _load_strategies(self) -> Dict:
        """Load strategy database"""
        default_strategies = {
            "early_game": {
                "build_advice": [
                    "Tag Small Guns, Lockpick, and Speech for versatility",
                    "Prioritize Agility (for AP) and Intelligence (for skill points)",
                    "Get Leather Armor as soon as possible",
                    "Stock up on Stimpaks - at least 5 before leaving town"
                ],
                "warnings": [
                    "Don't go to Necropolis too early - Super Mutants are tough",
                    "Avoid The Glow without Rad-X and RadAway",
                    "Save before important conversations (can't reload in ironman)"
                ],
                "optimal_path": "Vault 13 → Shady Sands → Vault 15 → Junktown → Hub → Necropolis"
            },
            "Shady Sands": {
                "optimal_path": "Rescue Tandi first (easy quest, good reputation boost)",
                "warnings": [
                    "Radscorpions have poison - bring antidote",
                    "Don't anger Aradesh - he's important for reputation"
                ],
                "build_advice": [
                    "Recruit Ian here - good early companion",
                    "Buy rope from general store - needed for some areas"
                ],
                "tactical_suggestions": [
                    "Kill radscorpions for XP before leaving",
                    "Talk to everyone to get location information",
                    "Trade for better armor if possible"
                ]
            },
            "Junktown": {
                "optimal_path": "Side with Killian (law) over Gizmo (crime) for better long-term outcome",
                "warnings": [
                    "Don't steal in Killian's store - heavily watched",
                    "Gizmo has guards - don't attack without preparation",
                    "Skulz gang can be dangerous at night"
                ],
                "build_advice": [
                    "Recruit Tycho - excellent companion for combat",
                    "Buy better weapons from Killian's store",
                    "Get Combat Leather Armor if you can afford it"
                ],
                "tactical_suggestions": [
                    "Do Killian's quest to bust Gizmo - good reward",
                    "Help Lars catch the Skulz thief",
                    "Stock up on ammo before leaving"
                ]
            },
            "The Hub": {
                "optimal_path": "Visit Water Merchants first, then explore other areas",
                "warnings": [
                    "Thieves Circle is dangerous - high chance of theft",
                    "Decker's assassinations have consequences",
                    "Irwin's farmhouse quest is harder than it seems"
                ],
                "build_advice": [
                    "This is best place to buy/sell equipment",
                    "Get better armor here before going to dangerous areas",
                    "Stock up on ammo - prices are reasonable"
                ],
                "tactical_suggestions": [
                    "Do missing caravan quest for good XP",
                    "Talk to Harold for Master information",
                    "Buy water from merchants to extend deadline"
                ]
            },
            "Necropolis": {
                "optimal_path": "Sneak past Super Mutants to get Water Chip from Vault 12",
                "warnings": [
                    "CRITICAL: Super Mutants in watershed are very tough",
                    "Don't anger Set - he controls access",
                    "Radiation in some areas - bring RadAway"
                ],
                "build_advice": [
                    "High Sneak skill makes this much easier",
                    "Bring at least 10 Stimpaks",
                    "Combat Armor recommended for fighting mutants"
                ],
                "tactical_suggestions": [
                    "Use Stealth Boy if you have one",
                    "Run away from Super Mutants if caught",
                    "Get Water Chip, fix pump for ghouls, then leave quickly"
                ]
            },
            "Brotherhood of Steel": {
                "optimal_path": "Complete initiation quest to gain access to equipment",
                "warnings": [
                    "Glow is EXTREMELY radioactive - mandatory Rad-X and RadAway",
                    "Bring at least 10 Rad-X and 10 RadAway for Glow",
                    "Brotherhood has strict rules - don't steal"
                ],
                "build_advice": [
                    "Join Brotherhood for Power Armor (best armor in game)",
                    "Brotherhood weapons are expensive but powerful",
                    "Scribe Vree has important lore information"
                ],
                "tactical_suggestions": [
                    "Do Glow quest ASAP to access Brotherhood inventory",
                    "Buy Plasma Rifle or Laser Rifle here",
                    "Power Armor makes you nearly invincible"
                ]
            },
            "combat": {
                "general_tactics": [
                    "Aimed shots to eyes for critical hits",
                    "Use cover and distance with ranged weapons",
                    "Let companions tank while you shoot",
                    "Save burst fire for groups of weak enemies"
                ],
                "enemy_specific": {
                    "Radscorpions": "Poison is dangerous early. Target eyes. Use ranged weapons.",
                    "Super Mutants": "Very tough. Use Plasma/Laser weapons. Aim for eyes or groin.",
                    "Deathclaws": "EXTREMELY DANGEROUS. Avoid until high level. Plasma Rifle recommended.",
                    "Raiders": "Weak early game. Good for XP farming.",
                    "Robots": "Resistant to lasers. Use Plasma or armor-piercing rounds."
                },
                "weapon_progression": [
                    "Level 1-5: 10mm Pistol, then Hunting Rifle",
                    "Level 6-10: Combat Shotgun or Assault Rifle",
                    "Level 11-15: Plasma Rifle or Laser Rifle",
                    "Level 16+: Turbo Plasma Rifle or Gatling Laser"
                ]
            },
            "skill_recommendations": {
                "essential": [
                    "Small Guns: Primary damage skill for early/mid game",
                    "Energy Weapons: Best weapons late game",
                    "Lockpick: Access to loot and shortcuts",
                    "Speech: Peaceful solutions, better prices",
                    "First Aid: Healing without using items"
                ],
                "useful": [
                    "Science: Access to computers and robots",
                    "Repair: Fix things, maintain equipment",
                    "Sneak: Avoid combat, surprise attacks",
                    "Doctor: Heal severe injuries and radiation"
                ],
                "optional": [
                    "Steal: Risky but profitable",
                    "Traps: Situational use",
                    "Barter: Speech usually better",
                    "Gambling: Money making (but risky)"
                ]
            }
        }
        
        return default_strategies
    
    def get_strategy_for_location(self, location: str, player_level: int) -> Optional[StrategyAdvice]:
        """Get strategy advice for current location"""
        if not self.enabled:
            return None
        
        # Determine strategy based on location
        strategy_key = None
        for key in self.strategies.keys():
            if key.lower() in location.lower() or location.lower() in key.lower():
                strategy_key = key
                break
        
        # Default to early game advice if no specific location
        if not strategy_key:
            strategy_key = "early_game" if player_level < 6 else "mid_game"
        
        strat_data = self.strategies.get(strategy_key, {})
        
        return StrategyAdvice(
            optimal_path=strat_data.get('optimal_path', 'No specific path recommended'),
            warnings=strat_data.get('warnings', []),
            build_advice=strat_data.get('build_advice', []),
            tactical_suggestions=strat_data.get('tactical_suggestions', [])
        )
    
    def get_combat_strategy(self, enemy_type: str) -> str:
        """Get specific combat advice"""
        if not self.enabled:
            return ""
        
        combat_data = self.strategies.get('combat', {})
        enemy_specific = combat_data.get('enemy_specific', {})
        
        for key, advice in enemy_specific.items():
            if key.lower() in enemy_type.lower():
                return advice
        
        return "Use tactical approach: cover, distance, aimed shots."
    
    def get_build_recommendations(self, current_level: int, current_skills: Dict[str, int]) -> List[str]:
        """Get character build recommendations"""
        if not self.enabled:
            return []
        
        skill_data = self.strategies.get('skill_recommendations', {})
        recommendations = []
        
        # Check essential skills
        essential = skill_data.get('essential', [])
        for skill_desc in essential:
            skill_name = skill_desc.split(':')[0].strip()
            if current_skills.get(skill_name, 0) < 100:
                recommendations.append(f"Prioritize: {skill_desc}")
        
        return recommendations[:5]  # Return top 5
    
    def to_prompt_text(self, location: str, player_level: int, in_combat: bool, nearby_enemies: List[str]) -> str:
        """Generate strategy context for LLM prompt"""
        if not self.enabled:
            return "\n=== STRATEGY CONTEXT ===\n[DISABLED - Playing without optimization hints]"
        
        text = "\n=== STRATEGY CONTEXT (OPTIMIZATION HINTS) ==="
        
        # Get location strategy
        strategy = self.get_strategy_for_location(location, player_level)
        if strategy:
            text += f"\n\nOPTIMAL PATH:\n{strategy.optimal_path}"
            
            if strategy.warnings:
                text += "\n\n⚠️  WARNINGS:"
                for warning in strategy.warnings:
                    text += f"\n  - {warning}"
            
            if strategy.build_advice:
                text += "\n\n💡 BUILD ADVICE:"
                for advice in strategy.build_advice:
                    text += f"\n  - {advice}"
            
            if strategy.tactical_suggestions:
                text += "\n\n🎯 TACTICAL SUGGESTIONS:"
                for suggestion in strategy.tactical_suggestions:
                    text += f"\n  - {suggestion}"
        
        # Add combat-specific advice if in combat
        if in_combat and nearby_enemies:
            text += "\n\n⚔️  COMBAT ADVICE:"
            combat_tactics = self.strategies.get('combat', {}).get('general_tactics', [])
            for tactic in combat_tactics[:3]:
                text += f"\n  - {tactic}"
            
            # Enemy-specific advice
            for enemy in nearby_enemies[:2]:
                enemy_name = enemy.get('name', '')
                advice = self.get_combat_strategy(enemy_name)
                if advice:
                    text += f"\n  - {enemy_name}: {advice}"
        
        return text
