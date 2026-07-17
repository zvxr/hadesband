# Dungeon Generation Plan

This document tracks dungeon-generation work that is separate from the
proper-noun reskin pass.

## Current Organic Terrain

- Implemented terrain family:
  - `tree`: blocking, choppable, organic, flammable.
  - `wood`: blocking, choppable, organic, flammable.
  - `vegetation`: blocks non-flying movement, choppable, organic, flammable.
  - `soil`: passable floor-like removed vegetation, organic.
- Implemented room-template symbols:
  - `v`: vegetation.
  - `t`: tree.
  - `w`: wood.
  - `m`: soil with a mushroom, flying monster, or nothing.
- Implemented chopping with `CHOP_1` and `CHOP_2`, using the same base
  formula as digging with organic-specific item bonuses.
- Implemented `FLY` passability for vegetation and lava-like terrain.
- Implemented a depth-10 Forest label for early manual testing.

## Room Selection Facts

- `room_template.txt` currently uses `type:1` for all active room templates.
  The old `type` field is effectively a selector for `random_room_template()`,
  but generation only asks for type 1.
- Room template ratings currently count as:
  - rating 1: 430
  - rating 2: 66
  - rating 3: 22
  - rating 4: 18
- Organic template counts currently are:
  - rating 1: 13
  - rating 2: 3
  - rating 3: 2
- Rating 4 is rare through `dungeon_profile.txt` cutoffs. Long-term, rating 4
  should be deprecated or reserved for unusual legacy behavior.
- Specific room templates are selected uniformly from all templates matching
  type and rating. There is no per-template weight.
- If a selected template fails to build, generation falls back to the existing
  profile retry path; it does not immediately try another template from the same
  rating pool.

## Target Model

Use room tags for semantic grouping and theme selection, while preserving normal
profile rating and rarity behavior.

Example room-template metadata:

```text
tags:organic | grove
```

Example level-theme metadata:

```text
theme:Forest
label:Forest
label-color:g
spawn:10:100:100:100
seed-room-tag:organic:80
```

The same infrastructure is also used by Round Halls, which appears at depths
6 and 66 and prefers `round` templates 66% of the time.

Semantics:

- Before normal room-profile selection, active level themes may roll their tag
  preferences.
- If a tag preference hits, room generation first tries to place rooms/templates
  whose tag set includes that tag.
- Normal rating and rarity behavior should still apply inside that restricted
  pool.
- If no tagged room can be placed, fall back to normal room selection.
- Do not use terrain bloom/post-processing for Forest cohesion. If stronger
  floor-wide identity is needed later, revisit deeper generation logic, possibly
  including a real use for room/template type or a new level generation mode.

## Forest Room Direction

The long-term goal is not only isolated grove rooms. We want forested versions
of classic dungeon shapes:

- simple chambers
- large rooms
- circular and oval rooms
- maze-like thickets
- treasure caches enclosed by tree or wood
- nests or vault-like setpieces with organic borders

Baseline organic frequency should come from the room-template pool itself. For
example, targeting 3% to 5% organic templates per rating means roughly:

- rating 1: about 13 to 22 organic templates.
- rating 2: about 2 to 4 organic templates.
- rating 3: about 1 organic template.

The Forest theme can then bias toward `organic` rooms without requiring all
organic rooms to be special-case generation.

## Dungeon Store Theme Direction

Dungeon store spawn rules now use the same theme interface rather than terrain
feature `spawn:` directives.

Example:

```text
theme:Dungeon Store
label:Dungeon Store
label-color:y
spawn:20:5:50:100
spawn:60:5:50:100
seed-terrain:STORE_DUNGEON
```

Current implementation:

- `STORE_DUNGEON` is a terrain feature in `terrain.txt`.
- It has the `DUNGEON_ROOM` terrain flag, but spawn rules live in
  `level_theme.txt`.
- `src/gen-cave.c` has a small seed-terrain builder registry keyed by terrain
  feature.  `STORE_DUNGEON` currently routes to `build_dungeon_store_room()`.
- `level_theme_seed_terrain_spawns()` in `src/game-world.c` applies the theme
  `spawn:` chance by exact depth and player mode.
- The sidebar label checks the generated chunk for any present `seed-terrain`
  feature and shows that theme's label when found.

## Fixed Monsters and Quest Floors

Current implementation:

- `quest.txt` defines fixed quest monsters by name, level, race, and number.
- During level generation, `cave_generate()` checks `is_quest()` and places
  quest monsters after the dungeon builder succeeds.
- Current fixed quests are Zeus at depth 99 and Hades at depth 100.
- Depth 37 has several depth-37 elemental monsters and objects, but it is not
  currently a special fixed level in the same way as the quest floors.

Future theme direction:

```text
theme:Hades
label:Hades
label-color:v
spawn:100:100:100:100
dungeon-monster:Hades, Lord of the Underworld:1
```

This could eventually unify quest-style fixed monster placement with the level
theme system. That should be designed carefully because quest completion,
stairs/recall restrictions, and unique placement rules currently live in the
quest system.

## Implementation Steps

1. Add room-template tags to the parser and data model.
2. Add `organic` tags to current organic templates.
3. Reassign organic templates from rating 4 into rating 1 through 3 as
   appropriate.
4. Workshop and add enough rating-1 organic templates to reach a meaningful
   baseline frequency.
5. Add level-theme tag preferences with `seed-room-tag:<tag>:<percent>`. Done.
6. Route room selection through tag-restricted candidates when an active theme
   preference hits. Done.
7. Replace hard-coded seed terrain builders with a small registry keyed by
   terrain feature. Done.

## Open Questions

- Whether `dungeon-monster` belongs in the theme system, or should instead be
  a cleaner evolution of `quest.txt`.
