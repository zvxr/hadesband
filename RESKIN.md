# Hadesband Re-skin
- Angband is a roguelike that centers around J.R.R. Tolkien's work. The adventurer descends a procedurally generated dungeon to face and destroy Morgoth, who rules over the lowest floor of the dungeon.
- Hadesband re-skins the game such that the adventurer descends to face Hades, lord of the underworld.
- Some core fantasy elements remain in place, but where available races and classes are changed to better suit an ancient Greco-Roman mythos.
- Uniques are all rebranded, where most simply change to appropriate names (Morgoth >> Hades), while changing pertinent/identifying traits (Hades is a god, not human).
- When considering changes to races and monsters, we should try to preserve key elements, even if they are more fantasy setting as opposed to Greco-Roman mythology. e.g., Kobold, Goblin, Human will probably remain.
- When considering changes to monsters and monster groups, we should try to respect letters. For example, we must include Gods, which can replace Ghost's "G". Ghosts could then move to "v" as part of vortexes or "s" creating a spirit monster type.
- We want most aspects to be recognizable. We'd still have Fighter and Mages, but instead of Barbarian we'd have Gladiator. We would keep recognizable weapons, but may consider analogues. Spear would remain Spear, but Pike might be replaced with Dory or Javalin.
- As a first pass, we would avoid changing the number of weapons, spells, objects, etc., and only very lightly change effects and purpose. We are most interested in preserving the "feel" of the game and providing enough identical or near-identical context, so that what few changes there are, they can be intuited.

# Working Principles
- Favor data-only renames before mechanical rule changes.
- Preserve gameplay roles, depths, rarity, and letters unless a section explicitly decides otherwise.
- Prefer obvious mythic analogues over obscure references.
- Keep broad fantasy staples when replacing them would make gameplay less readable.
- Track open decisions here before implementation.

# Discoveries
- Core data lives in `lib/gamedata/*.txt`.
- Player races: `lib/gamedata/p_race.txt`.
- Player classes and spellbooks: `lib/gamedata/class.txt`.
- Monsters and uniques: `lib/gamedata/monster.txt`; monster groups in `monster_base.txt`, `pit.txt`, `summon.txt`.
- Objects: `object.txt`, `object_base.txt`, `artifact.txt`, `ego_item.txt`, `flavor.txt`.
- Magic/effects naming: `realm.txt`, `projection.txt`, `player_timed.txt`, `activation.txt`.
- Stores and dungeon features: `store.txt`, `terrain.txt`, `room_template.txt`, `vault.txt`, `dungeon_profile.txt`.
- Player-facing intro/death/help text: `lib/screens/*.txt`, `lib/help/*.txt`, `README.md`.
- Tile pref files contain many exact monster/object/race names and will need follow-up after data renames.

# Aspect Notes

## Frame Story
- Goal: descend into the underworld and defeat Hades.
- Morgoth should become Hades, Lord of the Underworld.
- Major unique names should draw from recognizable Greco-Roman myth, not only underworld figures.
- Sauron should become Zeus, preferably `Zeus, God of the Sky`, as the level 99 questor.
- Preserve Sauron's quest role, depth, rarity, hit points, spell frequency, spell power, and broad threat profile.
- Small flavor spell swaps are acceptable when they preserve difficulty: prefer thunder/lightning over fire where there is a close existing spell equivalent.
- Initial Zeus spell target: change `BA_FIRE` to `BA_ELEC`; consider changing shape-form `BR_FIRE` to `BR_ELEC`.
- Sauron's shapechange maps well to Zeus mythos, but exact form names and mechanics should wait until a broader shapechange review.
- Sauron's current servant/friend uniques are Tolkien-specific; Khamûl and the Witch-King should be considered next.
- Khamûl is a depth 84 wraith warrior-king; `Ares, God of War` is a strong candidate if using recognizable gods.
- The Witch-King is a depth 85 wraith sorcerer/summoner; `Hecate, Goddess of Witchcraft` or `Thanatos, God of Death` are strong candidates, with Hecate best matching the magic role.
- Need decide whether final floor text, quests, victory messages, and death text mention Hades explicitly.

## Races
- Likely keep Human, Kobold, Goblin, Half-Orc style fantasy staples unless they clash strongly.
- Discuss replacing Tolkien-coded races: Hobbit, Dunadan, High-Elf.
- Barbarian likely becomes Gladiator or another martial culture.
- Fae may remain if framed as nymph/sprite-adjacent.
- Need preserve stats/XP unless a name implies changed mechanics.

## Classes
- Keep direct archetypes: Warrior/Fighter, Mage, Rogue, Ranger likely remain readable.
- Discuss Priest/Paladin/Necromancer/Druid naming in Greco-Roman terms.
- Red Mage can remain a fantasy class or become a named hybrid archetype.
- Avoid changing spell lists until class names/themes are settled.

## Monsters
- Biggest surface area; handle by families and unique tiers.
- Keep readable generic monsters: animals, undead, demons, dragons, trolls, orcs, goblins.
- Replace Tolkien uniques with mythic, heroic, divine, or underworld figures.
- Consider `G` for Gods; relocate Ghosts if needed.
- Respect display letters where practical.

## Objects
- Most mundane weapons/armor stay.
- Rename culturally specific weapons where clear: Pike -> Dory/Javelin candidate; keep Spear.
- Artifacts need the most care; preserve powers while replacing Tolkien identity.
- Consumables probably stay mostly readable.

## Magic
- Preserve effects and spell function first.
- Rename divine books/spells away from Valar/Tolkien terms.
- Decide whether realms map to Olympian/Chthonic/Nature/Arcane framing.

## Stores and Town
- Dungeon Organics can remain or be reframed later.
- Town store owner names should eventually shift to Greco-Roman or underworld flavor.
- Store behavior should not change in first pass.
- Open terminology: consider renaming player-facing "town" to "polis"; keep code/data identifiers as-is unless needed.

## Text, UI, and Help
- Splash/news and death screens should match Hadesband premise.
- Help docs likely retain many Angband references; update only player-visible lore first.
- README already describes Tactical Angband base; keep preserved history separate.
- If using "polis", audit UI/help text for places where "town" is literal gameplay terminology versus flavor.

## Tiles and Visuals
- Tile pref files map exact names; data renames may break tile assignments.
- After monsters/objects are renamed, update tile prefs in a focused pass.
- Icon file is renamed but art content is unchanged.

# First-Pass Plan
- Create mapping tables before edits: races, classes, monster bases, uniques, artifacts, books.
- Start with low-risk visible lore: final boss, news/death text, obvious Tolkien book/spell names.
- Then handle player options: races/classes.
- Then monster bases and uniques.
- Then objects/artifacts/ego names.
- Finish with tile prefs and help docs.

# Ready to Implement
- None yet; discuss each aspect before edits.

# Completed
- Project/product rename to Hadesband.
