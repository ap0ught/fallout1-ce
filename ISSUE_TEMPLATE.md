### Feature Request: Party Order Management System

**Description**
Implement a party order system that allows players to customize the order and positioning of party members during combat and exploration, similar to the Fallout 2 Party Orders mod.

**Motivation**
The current party system in Fallout 1 CE has a fixed ordering where party members are arranged by the order they join the party. This limits tactical gameplay options. Implementing a customizable party order would allow players to:
- Position tanky/melee characters in front to absorb damage
- Keep ranged/weak characters in the back for safety
- Organize party based on tactical preferences
- Match party order to visual formation on screen

**Related Systems**
- [x] Party System (src/game/party.h/.cc)
- [ ] Combat System (src/game/combat.*)
- [ ] Formation System
- [ ] UI/Party Screen

**Implementation Details**

**Changes Made:**
1. **Extended party.h** - Added four new functions:
   - `int partyMemberSetOrder(Object* object, int order)` - Set custom order for party member
   - `int partyMemberGetOrder(Object* object)` - Get current order of party member  
   - `void partyMemberApplyOrder()` - Apply current ordering to party formation
   - `void partyMemberResetOrder()` - Reset to default join order

2. **Enhanced party.cc** - Added order tracking and management:
   - `partyMemberOrder[20]` array to track custom ordering
   - `partyOrderCustomized` flag to track if custom order is active
   - Modified `partyMemberAdd()` to initialize new members with default order
   - Modified `partyMemberRemove()` to maintain order consistency
   - Enhanced `partyMemberSave()` and `partyMemberLoad()` to persist order across saves
   - Enhanced `partyMemberClear()` to reset order when clearing party
   - Added the four new order management functions with proper validation

**Technical Details:**
- Order 0 is reserved for the player character (fixed)
- Orders 1-19 are available for follower positions
- Functions include validation to prevent invalid orders
- Saving/loading preserves custom party order through game sessions
- Default behavior unchanged when no custom order is set

**Testing Plan:**
1. Verify party members can be assigned custom orders via console/commands
2. Confirm saved games maintain party order after loading
3. Ensure default behavior works when no custom order is set
4. Test edge cases (empty party, single member, max party size)
5. Verify no breaking changes to existing party functionality
6. Test order persistence through save/load cycles

**Related Issues**
- Inspired by: https://github.com/BGforgeNet/Fallout2_Party_Orders

**Additional Context**
This implementation follows the existing code patterns in the Fallout 1 CE codebase and maintains backward compatibility. The party order system can be expanded in the future to integrate more deeply with combat positioning and formation systems.