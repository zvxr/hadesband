# Hadesband Re-skin
- Angband is a roguelike that centers around J.R.R. Tolkien's work. The adventurer descends a procedurally generated dungeon to face and destroy Morgoth, who rules over the lowest floor of the dungeon.
- Hadesband re-skins the game such that the adventurer descends to face Hades, lord of the underworld.
- Some core fantasy elements remain in place, but where available races and classes are changed to better suit an ancient Greco-Roman mythos.
- Uniques are all rebranded, where most simply change to appropriate names (Morgoth >> Hades), while changing pertinent/identifying traits (Hades is a god, not human).
- When considering changes to races and monsters, we should try to preserve key elements, even if they are more fantasy setting as opposed to Greco-Roman mythology. e.g., Kobold, Goblin, Human will probably remain.
- When considering changes to monsters and monster groups, we should try to respect letters. For example, we must include Gods, which can replace Ghost's "G". Ghosts could then move to "v" as part of vortexes or "s" creating a spirit monster type.
- We want most aspects to remain recognizable. Race and class reskins should
  stay in the correct character-choice layer;
  familiar weapons remain unless a clear Greco-Roman analogue improves flavor.
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
- Terrain `spawn` rules for dungeon rooms are exact-depth and mode-aware:
  `spawn:<floor>:<classic %>:<recall %>:<nightmare %>`.
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
- Khamûl should become `Ares, God of War`.
- The Witch-King should become `Hecate, Goddess of Witchcraft`; changing `MALE` to `FEMALE` is acceptable.
- Need decide whether final floor text, quests, victory messages, and death text mention Hades explicitly.

## Races
- Likely keep Human, Kobold, Goblin, Half-Orc style fantasy staples unless they clash strongly.
- Discuss replacing Tolkien-coded races: Hobbit and High-Elf.
- Removed: Spartan was mistakenly added as a race; revisit Barbarian as a
  class/theme discussion instead.
- Done: Half-Giant renamed to Cyclops.
- Cyclops uses `x` for Cyclopean Rage: uses a 150-turn cooldown, alarms
  monsters, grants berserk combat effects, temporary shard resistance, and
  running for the same duration.
- Hadesband command convention: `s` invokes a class Skill and `x` invokes a
  racial Expertise.
- Active Skills and Expertise use the registry in `player-properties.c`;
  ownership checks class/race `pflags` directly, not derived player flags.
- Removed Tactical's original-keyset `x -> w0` weapon-swap keymap so `x`
  consistently invokes racial Expertise.
- Kobold uses `x` for Scurry: 20 turns of extra movement, then a 120-turn
  cooldown.
- Fae replaced by Siren, with a shared-cooldown Siren Song Expertise.
- Need preserve stats/XP unless a name implies changed mechanics.

| Tactical race | Hadesband race | Reskin | Balance |
| --- | --- | --- | --- |
| Human | Human | Pending; likely retain | Not reviewed |
| Half-Elf | Half-Elf | Pending | Reviewed; 110% XP |
| Elf | Elf | Pending | Not reviewed |
| Hobbit | Hobbit | Pending | Not reviewed |
| Gnome | Gnome | Pending | Not reviewed |
| Dwarf | Dwarf | Pending; likely retain | Not reviewed |
| Kobold | Kobold | Done; likely retain | Reviewed; Scurry, 120% XP |
| High-Elf | High-Elf | Pending | Not reviewed |
| Half-Orc | Half-Orc | Pending; likely retain | Not reviewed |
| Half-Troll | Half-Troll | Pending | Not reviewed |
| Barbarian | Barbarian | Pending; Spartan race removed | Not reviewed |
| Half-Giant | Cyclops | Done | Reviewed; Cyclopean Rage, 140% XP |
| New | Satyr | Done | Reviewed; 130% XP; Mushroom Lore, disenchantment resistance, Cure Confusion |
| New | Siren | Done | Reviewed; 125% XP; Siren Song |
| Dunadan | Demigod | Done | Reviewed; +2 all stats, 150% XP |

## Classes
- Keep direct archetypes: Warrior/Fighter, Mage, Rogue, Ranger likely remain readable.
- Discuss Priest/Paladin/Necromancer/Druid naming in Greco-Roman terms.
- Mystagogue replaced Red Mage as a mystery-cult warrior mage; mechanics retained.
- Avoid changing spell lists until class names/themes are settled.

| Tactical class | Hadesband class | Reskin | Balance |
| --- | --- | --- | --- |
| Warrior | Warrior | Pending; likely retain | Not reviewed |
| Mage | Mage | Pending; likely retain | Not reviewed |
| Druid | Druid | Pending | Not reviewed |
| Priest | Priest | Pending | Not reviewed |
| Necromancer | Necromancer | Pending | Not reviewed |
| Paladin | Paladin | Pending | Not reviewed |
| Rogue | Rogue | Pending; likely retain | Skill command migrated; balance not reviewed |
| Ranger | Ranger | Pending; likely retain | Not reviewed |
| New | Mystagogue | Done | Reviewed; INT-forward warrior-caster, stats +1/+2/-2/-1/+0, 20% XP |
| Blackguard | Stygian Warrior | Done | Display rename and class titles updated; mechanics retained |

## Monsters
- Biggest surface area; handle by families and unique tiers.
- Keep readable generic monsters: animals, undead, demons, dragons, trolls, orcs, goblins.
- Replace Tolkien uniques with mythic, heroic, divine, or underworld figures.
- Consider `G` for Gods; relocate Ghosts if needed.
- Respect display letters where practical.
- Monster `base` affects `friends-base` and `S_KIN`; monster base `glyph` affects ASCII/default display.
- Current `G` is `ghost`; current `W` is `wraith`; current `X` is `xorn`.
- Likely strategy: free `G` for `god`, move ghosts/spirits into `W` or another undead/spirit bucket, and eventually replace `xorn`/`X` with a more mythic earth/stone creature family.
- Existing `ainu` base and `S_AINU` summon/pit hooks are likely the safest migration path for a future `god` base and `S_GOD` naming.
- Added `god` as a new monster base with glyph `G`; old bases remain available during migration.
- Zeus, Ares, and Hecate now use `base:god`; Hades remains on the special final-boss base for now.
- Hecate's `S_KIN` now targets `base:god`; this is currently mostly inert until non-unique gods exist, but she still has other summoning spells.

## Monster Base/Glyph Migration
- Purpose: track monster family/glyph work separately from individual unique renames.
- Rule: create new bases first, then move monsters one by one; leave old bases available until their roster is intentionally migrated.
- Status: `god` exists with glyph `G`; Zeus, Ares, and Hecate use it.
- Status: `ghost` still exists with glyph `G`; existing ghosts/spirits have not been migrated.
- Status: `Morgoth` still exists with glyph `P`; Hades remains there for final-boss handling.
- Open: decide whether future god summons use `S_GOD`/`SUM_GOD` or keep old summon hooks until more gods exist.
- Open: decide whether ghosts move into `wraith`, a new `spirit` base, or another bucket.
- Open: decide replacement direction for `xorn`/`X`, likely a mythic earth/stone family.
- Planning: avoid over-splitting mythic taxonomy; prefer fitting myths into existing bases unless kin/summon behavior needs a distinct family.
- Planning: Cyclopes can likely use `giant`; demigods can use `person` for heroic mortals or `god` for divine-tier beings.
- Planning: demi-humans can usually use `humanoid`, `hybrid`, or specific creature bases; only create a new base if a repeatable summon/kin family emerges.
- Planning: centaurs, fauns, harpies, gorgons, and nymphs are candidates to map onto existing `hybrid`, `person`, `bird`, `snake`, `feline`, `tree`, or `elemental` buckets before adding new bases.
- Potential new bases: `spirit` for ghosts/shades if `wraith` is too narrow; `construct`/`automaton` for bronze guardians; `earthborn` for xorn-style stone/earth monsters.
- Low-hanging fit: harpies already use `hybrid`/`H`; this is a good model for winged demi-human monsters.
- Low-hanging fit: minotaur and manticore already use `hybrid`; keep unless a broader hybrid split becomes necessary.
- Current `hybrid` roster: white harpy, black harpy, hippogriff, griffon, chimaera, gorgimaera, manticore, minotaur, Baphomet the Minotaur Lord, jabberwock.
- Low-hanging fit: cyclops, lesser titan, and greater titan already use `giant`; keep Greco-Roman giants/titans there for now.
- Low-hanging fit: hydras already have a dedicated `hydra` base; keep.
- Open: consider expanding glyph `M` from `hydra` to broader `monstrous`/`monster` usage for minotaur, manticore, chimaera, etc.
- Caution: hydras have dedicated `HYDRA` summon, Hydra pit, and Serpents pit hooks; changing glyph only is low-risk, but merging hydras into `hybrid` would change summon/pit behavior unless those hooks are updated.
- Verified: duplicate glyphs are allowed; `base:hydra` could use glyph `H` while retaining `HYDRA` summon, hydra pits, and hydra kin behavior.
- Implemented: hydras keep `base:hydra` but now use glyph `H`; `HYDRA` summons, hydra pits, and hydra kin behavior remain base-driven.
- Implemented: `monstrous` exists with glyph `M`; hippogriff, griffon, chimaera, gorgimaera, manticore, minotaur, Baphomet, and jabberwock moved there.
- Implemented: harpies remain `base:hybrid` with glyph `H`.
- Hydra colors currently used: umber, orange, yellow, green, light green, red, light red, light purple.
- Hydra color gaps for future variants include white, slate, blue, light umber, purple, violet, mud, light yellow, magenta-pink, blue slate, and deep light blue; avoid black unless intentional because it blends into the background.
- Graphics note: tile prefs can map `monster:<exact name>` or `monster-base:<base>`; exact monster mappings override the visual impact of glyph/base changes in graphics mode.
- Graphics note: stale exact monster names in tile prefs are parser hazards after renames; missing mappings generally fall back to the monster's default attr/char.
- Implemented: bird pass renamed Tolkien/dark flyers to swan of Apollo, strix, bird of Ares, eagle of Zeus, and siren; Phoenix remains unchanged.
- Low-hanging fit: air/water/earth/fire spirits already use `elemental`; keep elemental spirits there unless we create a separate shade/ghost `spirit` base.
- Implemented: depth-43 elemental uniques now use primordial deity names: Aether, Pontus, Gaea, and Uranus.
- Elemental note: reserve Thalassa, Oceanus, Hemera, Erebus, Nyx, and Tartarus for future elemental/deep-underworld additions.
- Possible rename/family fit: `naga` can absorb Lamia/Echidna/Gorgon-like serpent-women; current gorgon already uses `naga`.
- Naga note: Medusa would fit here as a future unique, but is deferred beyond the initial reskin pass.
- Mold note: keep existing mold names and mechanics; `The Stygian Mold` is a possible future unique.
- Mushroom note: White Lotus and Black Lotus are possible future additions.
- Implemented: `tree`/`l` now means Living Wood; old tree/ent names became dryad, Daphne, greater dryad, blighted dryad, and ancient hamadryad.
- Implemented: `satyr`/`y` replaces yeeks; satyrs trade inherited acid immunity for confusion immunity, and Pan moves to depth 20 as a stronger trickster unique.
- Implemented: canine pass renamed Tolkien dogs/wolves to Argos, Maera, War dog, alpha wolf, guard dog of Hephaestus, Laelaps, Golden Dog, and Cerberus; generic wolves/werewolves/hellhounds remain.
- Implemented: giant uniques `Gilim` and `Nan` became Typhon and Antiphates; generic giants stay unchanged on `P`.
- Orc note: generic orcs remain unchanged; unique orcs are seeded with Dimension 20 easter-egg names.
- Possible future fit: centaurs and fauns probably use `hybrid`, `humanoid`, or `person` rather than new bases.

| Base | Glyph | Migration Status |
| --- | --- | --- |
| `ancient dragon` | `D` | Done for current unique pass; generic dragons retained |
| `ainu` | `A` | Old divine/spirit base; keep for now |
| `god` | `G` | Done for initial pass; Zeus/Ares/Hecate moved |
| `ant` | `a` | Done for current pass; myrmidon ant and Queen of the Myrmekes |
| `bat` | `b` | Keep base/mechanics; renamed bat of Gorgoroth and doombat |
| `bird` | `B` | Done for initial pass; Greek bird reskins complete |
| `canine` | `C` | Done for initial pass; Greek dog/wolf reskins complete |
| `centipede` | `c` | Done; keep existing names, glyph, and mechanics |
| `creeping coins` | `$` | Done; keep existing names, glyph, and mechanics |
| `dragon` | `d` | Done for current unique pass; generic dragons retained |
| `dragonfly` | `F` | Done for current pass; spelling normalized, mechanics unchanged |
| `eye` | `e` | Done for current pass; Stygian beholder and Argus renamed |
| `elemental` | `E` | Done for current unique pass; primordial elemental deity names added |
| `feline` | `f` | Done for current pass; The Nemean Lion renamed, generics retained |
| `ghost` | `G` | Untouched; conflicts with new god glyph |
| `giant` | `P` | Done for initial unique pass; generics retained |
| `golem` | `g` | Done for current pass; several constructs given mythic material names and Talos made unique |
| `humanoid` | `h` | Done for current pass; Greek humanoid generics and uniques reskinned |
| `hybrid` | `H` | Done for current pass; harpies remain here |
| `monstrous` | `M` | Done for current pass; major composite beasts moved here |
| `hydra` | `H` | Done for current pass; base retained, glyph changed from `M` |
| `ichor` | `i` | Done for current pass; base renamed from icky thing, glyph retained |
| `insect` | `I` | Done for current pass; pyrausta, gadfly, and Elysian firefly renamed |
| `jelly` | `j` | Done for current pass; kept generic, black ooze became Stygian ooze |
| `killer beetle` | `K` | Done; keep existing names, glyph, pits, and mechanics |
| `kobold` | `k` | Done for current pass; Mary Ann Skuttle renamed, generics retained |
| `lich` | `L` | Done for current pass; Mormo renamed, Vecna retained |
| `sphinx` | `x` | Done for current pass; base renamed from lurker, old tile coordinates reused |
| `major demon` | `U` | Done for current unique pass; Greek demon uniques renamed, generic demons retained |
| `mimic` | `?` | Done; keep existing names, descriptions, glyphs, and mechanics |
| `minor demon` | `u` | Done for current unique pass; Gello renamed, generic demons retained |
| `mold` | `m` | Done; keep existing names, glyph, and mechanics |
| `mushroom` | `,` | Done for current pass; light lotus descriptors added |
| `naga` | `n` | Done for current pass; temple naga and lamia renamed, gorgon retained |
| `ogre` | `O` | Done for current pass; Polyphemus renamed, generic ogres retained |
| `orc` | `o` | Done for current pass; generic orcs kept, uniques reskinned |
| `person` | `p` | Done for current pass; Greek person generics and uniques reskinned |
| `quadruped` | `q` | Done for current pass; Ayula, sacred deer, odontotyrannos, and Stygian mare renamed |
| `quylthulg` | `Q` | Pending |
| `reptile` | `R` | Done for current pass; The Tarrasque became Cetus, the Sea Beast |
| `rodent` | `r` | Done; keep existing names, glyph, and mechanics |
| `skeleton` | `s` | Done for current pass; Minos renamed, other skeletons retained |
| `snake` | `J` | Done for current pass; python, Pythian serpent, Stygian crawler, and Ophion serpent renamed |
| `spider` | `S` | Done for current pass; Tolkien names replaced, glyph/web/summon mechanics retained |
| `townsfolk` | `t` | Done for current pass; polis residents renamed, mechanics retained |
| `tree` | `l` | Done for initial pass; Living Wood/dryads complete |
| `troll` | `T` | Done for current pass; generics retained, uniques and underworld troll renamed |
| `vampire` | `V` | Done for current pass; Empusa renamed and Sauron shape cleaned to Vampire-Zeus |
| `vortex` | `v` | Done for current pass; Aeolus, storm of Lethe, and Darude sandstorm renamed |
| `worm` | `w` | Done for current pass; Stygian worm mass renamed, other worms retained |
| `wraith` | `W` | Pending; candidate home for some spirits |
| `xorn` | `X` | Pending; likely replacement candidate |
| `satyr` | `y` | Done for initial pass; yeeks replaced with satyrs and Pan |
| `yeti` | `Y` | Done; keep existing names, glyph, and mechanics |
| `zephyr hound` | `Z` | Done for current pass; lightning and underworld hound names added |
| `zombie` | `z` | Done for current pass; mummified cyclops added |
| `player` | `@` | Out of scope |
| `Morgoth` | `P` | Special; retained for Hades/final boss |

## Objects
- Most mundane weapons/armor stay.
- Rename culturally specific weapons only where clear; keep broad fantasy staples
  when readability is better than flavor.
- Artifacts need the most care; preserve powers while replacing Tolkien identity.
- Consumables probably stay mostly readable.
- Approval rule: propose item/object renames first, then implement only after
  explicit approval.

### Object Base Names
- Implemented weapon renames:
  `Short Sword` -> `Xiphos`,
  `Scimitar` -> `Kopis`,
  `Katana` -> `Machaira`,
  `Awl-Pike` -> `Javelin`.
- Implemented: `Javelin` has the `THROWING` flag.
- Implemented armor renames:
  `Wicker Shield` -> `Pelte`,
  `Small Metal Shield` -> `Aspis`,
  `Knight's Shield` -> `Spartan Shield`,
  `Metal Cap` -> `Bronze Cap`,
  `Metal Scale Mail` -> `Bronze Scale Armour`.
- Implemented starting-kit baseline: every class starts with `Soft Leather
  Armour`; every class starts with `Pelte` except Stygian Warrior, which keeps
  `Leather Shield`.

### Ego Items
- Minimal Olympian ego pass implemented:
  `of Westernesse` -> `of Ares`,
  `of Morgul` -> `of Hades`,
  `of Lothlórien` -> `of Artemis`,
  `of the Haradrim` -> `of Apollo`,
  `of Buckland` -> `of Hermes`.
- Wizard statistics and player combat docs were updated for the renamed egos.
- Deliberately retained for now: `Defender`, `Holy Avenger`, `Blessed`,
  `Elvenkind`, `(Dwarven)`, and other mechanically iconic ego names.

### Consumables
- Mushrooms reviewed separately; skip unless adding new mushroom content.
- Implemented food/drink renames:
  `Piece of Elvish Waybread` -> `Ambrosia`,
  `Sip of Miruvor` -> `Sip of Nectar`,
  `Swig of Orcish Liquor` -> `Swig of Spartan Moonshine`,
  `Scrap of Flesh` -> `Wild Boar`,
  `Flask of Whisky` -> `Minoan Wine`,
  `Draught of the Ents` -> `Draught of Gaia`.
- Honey-cake name retained; description changed only to remove the Beornings
  reference.
- Scroll names retained for readability. Implemented light Greek-flavored
  descriptions for `Phase Door`, `Teleportation`, `Teleport Level`,
  `Magic Mapping`, `Treasure Detection`, `Detect Invisible`, `Word of Recall`,
  and `Deep Descent`.
- Implemented potion rename: `Slime Mold Juice` -> `Herbal Tonic`.
- Potion names otherwise retained for readability. Implemented light
  Greek-flavored descriptions for `Speed`, `Heroism`, `Berserk Strength`,
  `Boldness`, `True Seeing`, `Life`, and `Experience`.
- Implemented amulet renames:
  `the Magi` -> `the Oracle`,
  `Weaponmastery` -> `Ares`,
  `Trickery` -> `Hermes`.
- Ring and amulet description pass implemented for `Speed`, `Flames`, `Ice`,
  `Lightning`, `the Dog`, `Sustenance`, `Devotion`, `Hermes`, `Brilliance`,
  and amulet `Teleportation`.
- Wand, rod, and staff names retained for readability. Implemented light
  Greek-flavored descriptions for selected distinctive devices, including
  `Wonder`, `Dragon's Breath`, `Annihilation`, `the Magi`, `Holiness`,
  `Detection`, `Restoration`, and `Recall`.
- Lights, chests, dragon scale mail, and money/material names reviewed; retain
  current names for readability.
- Object review can continue later with artifacts and object flavor tables.

## Magic
- Preserve effects and spell function first.
- Rename divine books/spells away from Valar/Tolkien terms.
- Decide whether realms map to Olympian/Chthonic/Nature/Arcane framing.
- Implemented light-touch spell/book renames:
  `[Wrath of the Valar]` -> `[Wrath of Olympus]`,
  `Spear of Oromë` -> `Spear of Athena`,
  `Become Pukel-man` -> `Become Earthborn`,
  `Grond's Blow` -> `Titan's Blow`,
  `Fume of Mordor` -> `Stygian Fume`.

## Terrain and Generation
- Implemented organic terrain family: tree, wood, vegetation, and removed
  vegetation/soil.
- Implemented `FLY` passability for vegetation and lava-like terrain, and
  `CHOP_1`/`CHOP_2` weapon flags for removing organic obstacles.
- Implemented `FLY` carried-weight forgiveness: speed penalties treat the
  character as carrying 20.0 lb less, without changing displayed burden,
  inventory limits, or caster armor encumbrance.
- Implemented room template symbols:
  `v` vegetation,
  `t` tree,
  `w` wood,
  `m` soil with a mushroom/flying-monster/nothing roll.
- Implemented expanded rating-4 organic templates: `Small Grove`, `Small Grove
  Reversed`, `Overgrown Thicket A-H`, and `Large Grove A-D`, with normal
  dungeon-profile selection at about a 1% unusual-room slice.
- Implemented wood nests as an occasional procedural monster-nest variant,
  using wood enclosure walls and two organic decoration patterns while keeping
  the usual `pit.txt` nest monster themes.
- Implemented a hard-coded depth-10 organic bloom for early manual testing;
  it affects eligible non-vault room floors only, leaving corridors and stairs
  untouched.

## Stores and Town
- Dungeon Organics can remain or be reframed later.
- Town store owner names should eventually shift to Greco-Roman or underworld flavor.
- Store behavior should not change in first pass.
- Open terminology: consider renaming player-facing "town" to "polis"; keep code/data identifiers as-is unless needed.
- Implemented Greek-flavored town store owner names for General Store, Armoury,
  Weaponsmith, Bookseller, Alchemy Shop, Magic Shop, and Black Market.
- Dungeon Organics owner names intentionally retained.
- Store owner purse values increased from the old `5000`-to-`30000` scale to
  the new `10000`-to-`40000` scale.

## Artifacts
- Artifact mechanics should be preserved unless explicitly discussed.
- Implemented first artifact batch:
  `Phial of Galadriel` -> `Phial of Selene`,
  `Star of Elendil` -> `Star of Apollo`,
  `Arkenstone of Thráin` -> `Omphalos of Delphi`,
  `Grond` -> `Titan's Hammer`,
  `Massive Iron Crown of Morgoth` -> `Massive Iron Crown of Hades`.
- Implemented sword and dagger artifact reskins, preserving base objects and
  mechanics while replacing most Tolkien-coded names with Greek mythic names.
- Implemented blunt weapon artifact reskins, preserving mechanics and retaining
  `of Orpheus` while refreshing its description.
- Implemented polearm artifact reskins, preserving mechanics and retaining
  `Glaive of Pain`.
- Implemented missile weapon artifact reskins, preserving mechanics.
- Implemented digging tool and boot artifact reskins, preserving mechanics and
  retaining `Stormwalker` while replacing its description.
- Implemented helmet and crown artifact reskins, preserving mechanics.
- Implemented light and heavy armor artifact reskins, preserving mechanics:
  `Hithlomir` -> `Kevin Sorbo's Leather Armor`,
  `of Himring` -> `of Thebes`,
  `Thalkettoth` -> `Hermes' Scales`,
  `of Arvedui` -> `of Theseus`,
  `of Caspanion` -> `of Pythagoras`,
  `of the Rohirrim` -> `of the Spartans`,
  `of Isildur` -> `of Caesar`,
  `Belegennon` -> `Aegis Mail`,
  `of Celeborn` -> `of Heracles`.
- Implemented cloak and glove artifact reskins, preserving mechanics:
  `Colluin` -> `Mantle of Olympus`,
  `Holcolleth` -> `Mantle of Hypnos`,
  `of Thingol` -> `of Ariadne`,
  `of Thorongil` -> `of Odysseus`,
  `Colannon` -> `Iris' Mantle`,
  `of Lúthien` -> `of Eurydice`,
  `of Tuor` -> `of Poseidon`,
  `Cambeleg` -> `Heracles' Grip`,
  `Cammithrim` -> `Apollo's Hands`,
  `Paurhach` -> `Hephaestus' Grip`,
  `Paurnimmen` -> `Boreas' Grip`,
  `Pauraegen` -> `Zeus' Grasp`,
  `Paurnen` -> `Hydra's Touch`,
  `Camlost` -> `Midas' Regret`,
  `of Fingolfin` -> `of Achilles`.
- Implemented shield artifact reskins, preserving mechanics:
  `of Elros` -> `of Asclepius`,
  `of Thorin` -> `of Atlas`,
  `of Celegorm` -> `of Artemis`,
  `of the Haradrim` -> `of Ares`,
  `of Anárion` -> `of Themis`,
  `of Eärendil` -> `of Apollo`,
  `of Gil-galad` -> `of Helios`.
- Implemented ring artifact reskins, preserving mechanics and base ring kinds:
  `of Barahir` -> `of Medusa`,
  `of Tulkas` -> `of Heracles`,
  `Narya` -> `Hestia's Flame`,
  `Nenya` -> `Persephone's Tear`,
  `Vilya` -> `Ouranos' Sapphire`,
  `The One Ring` -> `Ring of Gyges`.
- Implemented amulet artifact reskins, preserving mechanics and base amulet
  kinds:
  `of Carlammas` -> `of Hecate`,
  `of Ingwë` -> `of Athena`,
  `of the Dwarves` -> `of Hephaestus`,
  `Elessar` -> `Aesonstone`,
  `Evenstar` -> `Pomegranate Jewel`.
- Reviewed dragon scale mail artifacts and retained `Razorback`, `Mediator`,
  and `Bladeturner`; lightly polished descriptions only.
- Applied base object description-only cleanup for `Battle Axe`, `Great Axe`,
  and `Elven Cloak`; no object names or mechanics changed.
- Updated remaining store owner race labels:
  `Cephalus the Hospitable (Hobbit)` -> `(Satyr)`,
  `Melampus the Beast-Master (Hobbit)` -> `(Satyr)`,
  `Circe the Shifting (Hobbit)` -> `(Human)`.
- Applied birth history text-only Greek flavor pass for human social origins,
  half-elf/elf/high-elf ancestry, smallfolk history, and dwarf forge-clan
  backgrounds; no chart structure, race links, or probabilities changed.
- Renamed dungeon level labels in `world.txt` from `Angband N` to
  `Underworld N`; retained `Town`, `None`, and the existing level graph.
- Replaced the intro `news.txt` Angband ASCII art and old Tolkien quote with
  horizontal two-font Hadesband ASCII art fitted to the 80x23 splash screen
  assumption.
- Renamed food consumable `Handful of Dried Fruits` to `Wild Morels`, keeping
  its normal food mechanics unchanged.
- Began organic terrain support for future dungeon generation: added tree,
  wood, vegetation, and soil terrain, plus chopping, flight passability,
  flammability, and targeted clear-spawn hooks.

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
- None currently.

# Completed
- Project/product rename to Hadesband.
- Renamed four high-tier uniques: Morgoth -> Hades, Sauron -> Zeus, Khamûl -> Ares, Witch-King -> Hecate.
- Updated quest records, Zeus friend references, tile prefs, sound comment, Borg boss checks, and monster test lookup for those unique renames.
- Changed Zeus's direct `BA_FIRE` spell to `BA_ELEC`; deferred shape-form changes.
- User smoke-tested the four unique rename pass successfully.
- Added `god` monster base, moved Zeus/Ares/Hecate to it, and added Gods to monster knowledge.
- Reframed `tree` monsters as Living Wood: dryads, Daphne, blighted dryad, and ancient hamadryad.
