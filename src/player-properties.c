/**
 * \file player-properties.c 
 * \brief Class and race abilities
 *
 * Copyright (c) 1997-2020 Ben Harrison, James E. Wilson, Robert A. Koeneke,
 * Leon Marrick, Bahman Rabii, Nick McConnell
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */

#include "angband.h"
#include "player-properties.h"
#include "effects.h"
#include "game-input.h"
#include "game-world.h"
#include "init.h"
#include "mon-desc.h"
#include "mon-move.h"
#include "mon-predicate.h"
#include "mon-spell.h"
#include "mon-timed.h"
#include "mon-util.h"
#include "obj-desc.h"
#include "obj-make.h"
#include "obj-pile.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "player-calcs.h"
#include "player-timed.h"
#include "player-util.h"
#include "project.h"
#include "target.h"

typedef void (*player_power_handler)(int dir);

struct player_power {
	int flag;
	enum player_power_source source;
	const char *name;
	bool needs_direction;
	player_power_handler handler;
};

static void use_steal(int dir);
static void use_mana_siphon(int dir);
static void use_prayer(int dir);
static void use_cyclopean_rage(int dir);
static void use_kobold_scurry(int dir);
static void use_siren_song(int dir);
static void use_war_cry(int dir);
static void use_cure_confusion(int dir);
static void use_stone_lore(int dir);
static void use_demigod_taunt(int dir);
static void use_fletch_ammo(int dir);

static const struct player_power player_powers[] = {
	{ PF_STEAL, PLAYER_POWER_CLASS, "Steal", true, use_steal },
	{ PF_MANA_STEAL, PLAYER_POWER_CLASS, "Siphon Mana", true,
		use_mana_siphon },
	{ PF_PRAYER, PLAYER_POWER_CLASS, "Prayer", false, use_prayer },
	{ PF_CYCLOPEAN_RAGE, PLAYER_POWER_RACE, "Cyclopean Rage", false,
		use_cyclopean_rage },
	{ PF_KOBOLD_SCURRY, PLAYER_POWER_RACE, "Scurry", false,
		use_kobold_scurry },
	{ PF_SIREN_SONG, PLAYER_POWER_RACE, "Siren Song", false,
		use_siren_song },
	{ PF_WAR_CRY, PLAYER_POWER_RACE, "War Cry", false, use_war_cry },
	{ PF_CURE_CONFUSION, PLAYER_POWER_RACE, "Cure Confusion", false,
		use_cure_confusion },
	{ PF_STONE_LORE, PLAYER_POWER_RACE, "Stone Lore", false,
		use_stone_lore },
	{ PF_DEMIGOD_TAUNT, PLAYER_POWER_RACE, "Taunt", false,
		use_demigod_taunt },
	{ PF_FLETCH_AMMO, PLAYER_POWER_RACE, "Fletch Ammo", true,
		use_fletch_ammo }
};

struct siren_song {
	const char *name;
	int level;
	int recharge;
	int fail;
	int effect;
	const char *desc;
};

static const struct siren_song siren_songs[] = {
	{ "Dread Song", 1, 25, 25, MON_TMD_FEAR, "frighten one target" },
	{ "Lullaby", 8, 35, 30, MON_TMD_SLEEP, "put one target to sleep" },
	{ "Impeding Song", 15, 45, 35, MON_TMD_SLOW, "slow one target" },
	{ "Distracting Song", 25, 60, 45, MON_TMD_CONF, "confuse one target" },
	{ "Siren's Song", 40, 120, 60, MON_TMD_COMMAND,
		"briefly command one target" }
};

struct stone_lore {
	const char *name;
	int level;
	int recharge;
	int effect1;
	int effect2;
	const char *desc;
};

static const struct stone_lore stone_lore[] = {
	{ "Ore Sense", 1, 25, EF_DETECT_ORE, EF_NONE, "detect nearby ore" },
	{ "Treasure Sense", 8, 40, EF_DETECT_ORE, EF_DETECT_GOLD,
		"detect ore and treasure" },
	{ "Hazard Sense", 12, 40, EF_DETECT_TRAPS, EF_NONE,
		"detect nearby traps" },
	{ "Passage Sense", 20, 40, EF_DETECT_DOORS, EF_DETECT_STAIRS,
		"detect doors and stairs" },
	{ "Deep Sense", 30, 400, EF_DETECT_OBJECTS, EF_NONE,
		"detect nearby objects" }
};

enum prayer_boon {
	PRAYER_MEND,
	PRAYER_RENEW_SPIRIT,
	PRAYER_BLESSING,
	PRAYER_CLEANSE,
	PRAYER_SANCTUARY,
	PRAYER_ELEMENTAL_AEGIS
};

static const char *prayer_boons[] = {
	"Mend",
	"Renew Spirit",
	"Blessing",
	"Cleanse",
	"Sanctuary",
	"Elemental Aegis"
};

static const char *prayer_descs[] = {
	"heal hit points",
	"restore spell points",
	"gain blessing",
	"cure ailments",
	"gain protection from evil",
	"gain one elemental resistance"
};

static bool player_owns_power(const struct player_power *power)
{
	if (power->source == PLAYER_POWER_CLASS) {
		return pf_has(player->class->pflags, power->flag);
	}

	return pf_has(player->race->pflags, power->flag);
}

static const struct player_power *find_player_power(
		enum player_power_source source)
{
	size_t i;

	for (i = 0; i < N_ELEMENTS(player_powers); i++) {
		if (player_powers[i].source == source &&
				player_owns_power(&player_powers[i])) {
			return &player_powers[i];
		}
	}

	return NULL;
}

const char *player_power_name(enum player_power_source source)
{
	const struct player_power *power = find_player_power(source);

	return power ? power->name : NULL;
}

bool player_power_needs_direction(enum player_power_source source)
{
	const struct player_power *power = find_player_power(source);

	return power && power->needs_direction;
}

void use_player_power(enum player_power_source source, struct command *cmd)
{
	const struct player_power *power = find_player_power(source);
	int dir = 0;

	if (!power) {
		msg(source == PLAYER_POWER_CLASS ?
			"You have no class skill to use." :
			"You have no racial expertise to use.");
		return;
	}

	if (power->needs_direction &&
			cmd_get_direction(cmd, "direction", &dir, false) != CMD_OK) {
		return;
	}

	power->handler(dir);
}

static struct monster *power_target_monster(int *dir)
{
	struct loc grid;
	struct monster *mon;

	player->upkeep->energy_use = z_info->move_energy;
	grid = loc_sum(player->grid, ddgrid[*dir]);
	mon = square_monster(cave, grid);

	if (player_confuse_dir(player, dir, false)) {
		grid = loc_sum(player->grid, ddgrid[*dir]);
		mon = square_monster(cave, grid);
	}

	return mon;
}

static void use_steal(int dir)
{
	struct monster *mon = power_target_monster(&dir);

	if (mon) {
		steal_monster_item(mon, -1);
	} else {
		msg("You spin around.");
	}
}

static void use_mana_siphon(int dir)
{
	struct monster *mon = power_target_monster(&dir);
	char m_name[80];
	int mlevel, spell_power, chance, roll;
	int missing_mana, mana;

	if (!mon) {
		msg("You spin around.");
		return;
	}

	monster_desc(m_name, sizeof(m_name), mon, MDESC_TARG);

	if (!monster_has_non_innate_spells(mon)) {
		msg("You sense no spell energy to steal from %s.", m_name);
		monster_wake(mon, false, 100);
		return;
	}

	if (player->csp >= player->msp) {
		msg("Your mana is already full.");
		return;
	}

	mlevel = MAX(1, mon->race->level);
	spell_power = MAX(mlevel, mon->race->spell_power);

	/* Device skill already includes class, race, level and INT. */
	chance = 50 + player->lev + player->state.stat_ind[STAT_INT] +
		(player->state.skills[SKILL_DEVICE] / 2);
	chance -= mlevel + (spell_power / 3);
	if (monster_is_unique(mon)) chance -= 20;
	chance = MIN(95, MAX(5, chance));

	roll = randint0(100);
	if (roll < chance) {
		missing_mana = player->msp - player->csp;
		mana = 1 + (player->lev / 5) + (mlevel / 10) +
			(spell_power / 20);
		mana = MIN(20, MIN(missing_mana, MAX(1, mana)));

		player_restore_mana(player, mana);
		msg("You steal %d mana from %s.", mana, m_name);
		monster_wake(mon, false, 100);
		return;
	}

	msg("You fail to siphon mana from %s.", m_name);
	monster_wake(mon, true, 100);
	monster_desc(m_name, sizeof(m_name), mon, MDESC_STANDARD);
	msg("%s cries out in anger!", m_name);
}

static int prayer_wis_bonus(void)
{
	return MAX(0, (player->state.stat_ind[STAT_WIS] - 10) / 2);
}

static bool prayer_cleanse(void)
{
	bool changed = false;

	changed |= player_clear_timed(player, TMD_AFRAID, true, false);
	changed |= player_clear_timed(player, TMD_POISONED, true, false);
	changed |= player_clear_timed(player, TMD_CONFUSED, true, false);
	changed |= player_clear_timed(player, TMD_CUT, true, false);
	changed |= player_clear_timed(player, TMD_STUN, true, false);

	return changed;
}

static void prayer_apply(enum prayer_boon boon)
{
	int wis = prayer_wis_bonus();
	int duration;
	bool ident = false;

	switch (boon) {
		case PRAYER_MEND: {
			int heal = 25 + player->lev * 3 + wis * 2;

			msg("A gentle mercy answers your prayer.");
			if (player->chp >= player->mhp) {
				msg("You feel no different.");
				return;
			}
			effect_simple(EF_HEAL_HP, source_player(), format("%d", heal),
				0, 0, 0, 0, 0, &ident);
			break;
		}

		case PRAYER_RENEW_SPIRIT: {
			int mana = 4 + player->lev / 4 + wis / 2;

			msg("Your spirit is renewed.");
			if (!player_restore_mana(player, mana)) {
				msg("You feel no different.");
			}
			break;
		}

		case PRAYER_BLESSING:
			duration = 20 + player->lev + wis;
			msg("A blessing settles over you.");
			(void)player_inc_timed(player, TMD_BLESSED, duration, true,
				false, false);
			break;

		case PRAYER_CLEANSE:
			msg("A cleansing light passes through you.");
			if (!prayer_cleanse()) {
				msg("You feel no different.");
			}
			break;

		case PRAYER_SANCTUARY:
			duration = 20 + player->lev * 2 + wis;
			msg("A sanctuary of faith surrounds you.");
			(void)player_inc_timed(player, TMD_PROTEVIL, duration, true,
				false, false);
			break;

		case PRAYER_ELEMENTAL_AEGIS: {
			static const int resists[] = {
				TMD_OPP_ACID,
				TMD_OPP_ELEC,
				TMD_OPP_FIRE,
				TMD_OPP_COLD,
				TMD_OPP_POIS
			};
			int resist = resists[randint0(N_ELEMENTS(resists))];

			duration = 20 + player->lev + wis;
			msg("An elemental aegis forms around you.");
			(void)player_inc_timed(player, resist, duration, true,
				false, false);
			break;
		}
	}
}

static void use_prayer(int dir)
{
	enum prayer_boon boon;
	int choices[N_ELEMENTS(prayer_boons)];
	int i;

	if (player->timed[TMD_PRAYER_COOLDOWN]) {
		msg("You need %d more turns before another prayer.",
			player->timed[TMD_PRAYER_COOLDOWN]);
		return;
	}

	for (i = 0; i < (int)N_ELEMENTS(choices); i++) {
		choices[i] = i;
	}

	if (player->lev >= 30) {
		const char *menu_choices[3];
		char entries[3][80];
		int choice;

		for (i = (int)N_ELEMENTS(choices) - 1; i > 0; i--) {
			int j = randint0(i + 1);
			int tmp = choices[i];

			choices[i] = choices[j];
			choices[j] = tmp;
		}

		for (i = 0; i < 3; i++) {
			strnfmt(entries[i], sizeof(entries[i]), "%-16s %s",
				prayer_boons[choices[i]], prayer_descs[choices[i]]);
			menu_choices[i] = entries[i];
		}

		choice = get_power_menu("Choose a divine boon: ", menu_choices, 3);
		if (choice < 0 || choice >= 3) {
			return;
		}
		boon = choices[choice];
	} else {
		boon = randint0(N_ELEMENTS(prayer_boons));
	}

	player->upkeep->energy_use = z_info->move_energy;
	msg("You pray for divine aid.");
	prayer_apply(boon);
	(void)player_set_timed(player, TMD_PRAYER_COOLDOWN, 120, true, false);
}

static void use_cyclopean_rage(int dir)
{
	int duration;

	if (player->timed[TMD_CYCLOPEAN_RAGE] || player->timed[TMD_SHERO]) {
		msg("You are already consumed by rage.");
		return;
	}

	if (player->timed[TMD_CYCLOPEAN_RAGE_COOLDOWN]) {
		msg("You need %d more turns before unleashing your Cyclopean rage again.",
			player->timed[TMD_CYCLOPEAN_RAGE_COOLDOWN]);
		return;
	}

	player->upkeep->energy_use = z_info->move_energy;
	(void)player_clear_timed(player, TMD_AFRAID, true, false);

	duration = 10 + randint1(10) + player->lev / 2;
	(void)player_inc_timed(player, TMD_CYCLOPEAN_RAGE, duration, true,
		false, false);
	(void)player_inc_timed(player, TMD_RUNNING, duration, true, false, false);
	(void)player_set_timed(player, TMD_CYCLOPEAN_RAGE_COOLDOWN, 150, true,
		false);
	monsters_handle_player_noise(100);
}

static void use_kobold_scurry(int dir)
{
	if (player->timed[TMD_SCURRY_COOLDOWN]) {
		msg("You need %d more turns before you can scurry again.",
			player->timed[TMD_SCURRY_COOLDOWN]);
		return;
	}

	player->upkeep->energy_use = z_info->move_energy;
	(void)player_set_timed(player, TMD_RUNNING, 20, true, false);
	(void)player_set_timed(player, TMD_SCURRY_COOLDOWN, 120, true, false);
}

static int siren_song_chance(const struct siren_song *song)
{
	int stat = (player->state.stat_ind[STAT_INT] +
		player->state.stat_ind[STAT_WIS]) / 2;
	int chance = song->fail;

	chance -= 3 * (player->lev - song->level);
	chance -= stat / 2;

	if (player_of_has(player, OF_AFRAID)) chance += 20;
	if (player->timed[TMD_CONFUSED]) chance += 15;
	if (player->timed[TMD_STUN] > 50) {
		chance += 25;
	} else if (player->timed[TMD_STUN]) {
		chance += 15;
	}

	return MIN(95, MAX(5, chance));
}

static int siren_song_duration(const struct siren_song *song)
{
	switch (song->effect) {
		case MON_TMD_FEAR:
			return 10 + player->lev;
		case MON_TMD_SLEEP:
			return 100 + player->lev * 10;
		case MON_TMD_SLOW:
			return 10 + player->lev / 3;
		case MON_TMD_CONF:
			return 8 + player->lev / 4;
		case MON_TMD_COMMAND:
			return 5 + randint1(5);
		default:
			return 0;
	}
}

static struct monster *siren_song_target(int dir)
{
	struct monster *mon = target_get_monster();
	struct loc path_g[256];
	struct loc target;
	int i, path_n;

	if (mon && target_able(mon) &&
			projectable(cave, player->grid, mon->grid, PROJECT_NONE)) {
		return mon;
	}

	target = loc(player->grid.x + ddgrid[dir].x * z_info->max_range,
		player->grid.y + ddgrid[dir].y * z_info->max_range);
	path_n = project_path(cave, path_g, z_info->max_range, player->grid,
		target, PROJECT_STOP);

	for (i = 0; i < path_n; i++) {
		mon = square_monster(cave, path_g[i]);
		if (mon && target_able(mon)) {
			return mon;
		}
	}

	return NULL;
}

static void use_siren_song(int dir)
{
	const struct siren_song *song;
	const char *choices[N_ELEMENTS(siren_songs)];
	char entries[N_ELEMENTS(siren_songs)][96];
	int choice_map[N_ELEMENTS(siren_songs)];
	struct monster *mon;
	char m_name[80];
	int choice, chance, duration, count = 0;
	size_t i;

	if (player->timed[TMD_SIREN_SONG_COOLDOWN]) {
		msg("You need %d more turns before your voice recovers.",
			player->timed[TMD_SIREN_SONG_COOLDOWN]);
		return;
	}

	for (i = 0; i < N_ELEMENTS(siren_songs); i++) {
		int fail = siren_song_chance(&siren_songs[i]);

		if (player->lev < siren_songs[i].level) continue;

		strnfmt(entries[i], sizeof(entries[i]),
			"%-18s Lv %2d  Rchg %3d  Fail %2d%%  %s",
			siren_songs[i].name, siren_songs[i].level,
			siren_songs[i].recharge, fail, siren_songs[i].desc);
		choices[count] = entries[i];
		choice_map[count++] = i;
	}

	choice = get_power_menu("Sing which song? ", choices,
		count);
	if (choice < 0 || choice >= count) {
		return;
	}

	song = &siren_songs[choice_map[choice]];
	if (song->effect == MON_TMD_COMMAND && player->timed[TMD_COMMAND]) {
		msg("You are already commanding a creature.");
		return;
	}

	if (!get_aim_dir(&dir)) {
		return;
	}

	mon = siren_song_target(dir);
	if (!mon) {
		msg("Your song finds no suitable target.");
		return;
	}

	monster_desc(m_name, sizeof(m_name), mon, MDESC_TARG);
	player->upkeep->energy_use = z_info->move_energy;
	(void)player_set_timed(player, TMD_SIREN_SONG_COOLDOWN,
		song->recharge, true, false);

	chance = siren_song_chance(song);
	if (randint0(100) < chance) {
		msg("Your voice falters before reaching %s.", m_name);
		return;
	}

	duration = siren_song_duration(song);

	if (song->effect == MON_TMD_COMMAND) {
		if (monster_is_unique(mon)) {
			msg("%s refuses to heed your song.", m_name);
			return;
		}
		if (randint1(player->lev) < randint1(mon->race->level)) {
			msg("%s resists your song.", m_name);
			return;
		}

		player_set_timed(player, TMD_COMMAND, duration, false, false);
		mon_inc_timed(mon, MON_TMD_COMMAND, duration, 0);
		return;
	}

	mon_inc_timed(mon, song->effect, duration, 0);
}

static void use_war_cry(int dir)
{
	struct monster *mon;
	char m_name[80];

	if (player->timed[TMD_WAR_CRY_COOLDOWN]) {
		msg("You need %d more turns before another war cry.",
			player->timed[TMD_WAR_CRY_COOLDOWN]);
		return;
	}

	if (!get_aim_dir(&dir)) {
		return;
	}

	mon = siren_song_target(dir);
	if (!mon) {
		msg("Your war cry finds no suitable target.");
		return;
	}

	monster_desc(m_name, sizeof(m_name), mon, MDESC_TARG);
	player->upkeep->energy_use = z_info->move_energy;
	(void)player_set_timed(player, TMD_WAR_CRY_COOLDOWN, 60, true,
		false);

	if (randint1(player->lev + 10) < randint1(mon->race->level + 10)) {
		msg("%s stands firm against your war cry.", m_name);
		return;
	}

	mon_inc_timed(mon, MON_TMD_FEAR, 10 + player->lev, 0);
}

static void use_cure_confusion(int dir)
{
	if (player->timed[TMD_CURE_CONFUSION_COOLDOWN]) {
		msg("You need %d more turns before clearing your thoughts again.",
			player->timed[TMD_CURE_CONFUSION_COOLDOWN]);
		return;
	}

	if (!player->timed[TMD_CONFUSED]) {
		msg("Your thoughts are already clear.");
		return;
	}

	player->upkeep->energy_use = z_info->move_energy;
	(void)player_clear_timed(player, TMD_CONFUSED, true, false);
	(void)player_set_timed(player, TMD_CURE_CONFUSION_COOLDOWN, 120,
		true, false);
}

static void use_stone_lore(int dir)
{
	const char *choices[N_ELEMENTS(stone_lore)];
	char entries[N_ELEMENTS(stone_lore)][96];
	int choice_map[N_ELEMENTS(stone_lore)];
	const struct stone_lore *lore;
	int choice, count = 0;
	size_t i;
	bool ident = false;

	if (player->timed[TMD_STONE_LORE_COOLDOWN]) {
		msg("You need %d more turns before reading the stone again.",
			player->timed[TMD_STONE_LORE_COOLDOWN]);
		return;
	}

	for (i = 0; i < N_ELEMENTS(stone_lore); i++) {
		if (player->lev < stone_lore[i].level) continue;

		strnfmt(entries[i], sizeof(entries[i]),
			"%-18s Lv %2d  Rchg %3d  %s",
			stone_lore[i].name, stone_lore[i].level,
			stone_lore[i].recharge, stone_lore[i].desc);
		choices[count] = entries[i];
		choice_map[count++] = i;
	}

	choice = get_power_menu("Use which stone lore? ", choices,
		count);
	if (choice < 0 || choice >= count) {
		return;
	}

	lore = &stone_lore[choice_map[choice]];

	player->upkeep->energy_use = z_info->move_energy;
	(void)player_set_timed(player, TMD_STONE_LORE_COOLDOWN,
		lore->recharge, true, false);

	effect_simple(lore->effect1, source_player(), "0", 0, 0, 0, 22, 40,
		&ident);
	if (lore->effect2 != EF_NONE) {
		effect_simple(lore->effect2, source_player(), "0", 0, 0, 0,
			22, 40, &ident);
	}
}

static void use_demigod_taunt(int dir)
{
	if (player->timed[TMD_DEMIGOD_TAUNT_COOLDOWN]) {
		msg("You need %d more turns before another divine challenge.",
			player->timed[TMD_DEMIGOD_TAUNT_COOLDOWN]);
		return;
	}

	player->upkeep->energy_use = z_info->move_energy;
	(void)player_inc_timed(player, TMD_TAUNT, 20 + player->lev / 2,
		true, false, false);
	(void)player_set_timed(player, TMD_DEMIGOD_TAUNT_COOLDOWN, 120,
		true, false);
	monsters_handle_player_noise(100);
}

static void use_fletch_ammo(int dir)
{
	const char *choices[] = { "Arrows", "Bolts" };
	int choice, tval, sval, quantity;
	struct object_kind *kind;
	struct object *ammo;
	struct loc grid;
	int feat;
	char o_name[80];

	if (player_confuse_dir(player, &dir, false)) {
		/* Direction may have changed. */
	}

	grid = loc_sum(player->grid, ddgrid[dir]);
	if (!square_in_bounds_fully(cave, grid)) {
		msg("There is no wood there to fletch.");
		return;
	}

	feat = square(cave, grid)->feat;
	if (feat != FEAT_WOOD && feat != FEAT_TREE) {
		msg("You need adjacent wood or a tree to fletch ammunition.");
		return;
	}

	choice = get_power_menu("Fletch which ammunition? ", choices,
		N_ELEMENTS(choices));
	if (choice < 0 || choice >= (int)N_ELEMENTS(choices)) {
		return;
	}

	tval = choice == 0 ? TV_ARROW : TV_BOLT;
	sval = lookup_sval(tval, choice == 0 ? "Arrow" : "Bolt");
	kind = lookup_kind(tval, sval);
	if (!kind) {
		msg("You cannot find a suitable pattern for that ammunition.");
		return;
	}

	ammo = object_new();
	object_prep(ammo, kind, 0, MAXIMISE);
	quantity = 10 + randint1(MAX(1, player->lev / 2));
	ammo->number = MIN(quantity, ammo->kind->base->max_stack);
	ammo->origin = ORIGIN_NONE;
	ammo->origin_depth = 0;

	player->upkeep->energy_use = z_info->move_energy;
	square_chop_terrain(cave, grid);
	if (cave->depth == 0) expose_to_sun(cave, grid, is_daytime());
	player->upkeep->update |= (PU_UPDATE_VIEW | PU_MONSTERS);
	player->upkeep->redraw |= (PR_MONLIST | PR_ITEMLIST);

	object_desc(o_name, sizeof(o_name), ammo, ODESC_BASE, player);
	msg("You fletch %s from the wood.", o_name);
	drop_near(cave, &ammo, 0, player->grid, true, true);
}

/**
 * ------------------------------------------------------------------------
 * Ability utilities
 * ------------------------------------------------------------------------ */
bool class_has_ability(const struct player_class *class,
					   struct player_ability *ability)
{
	if (streq(ability->type, "player") &&
		pf_has(class->pflags, ability->index)) {
		return true;
	} else if (streq(ability->type, "object") &&
			   of_has(class->flags, ability->index)) {
		return true;
	}
	return false;
}

bool race_has_ability(const struct player_race *race,
					  struct player_ability *ability)
{
	if (streq(ability->type, "player") &&
		pf_has(race->pflags, ability->index)) {
		return true;
	} else if (streq(ability->type, "object") &&
			   of_has(race->flags, ability->index)) {
		return true;
	} else if (streq(ability->type, "element") &&
			   (race->el_info[ability->index].res_level == ability->value)) {
		return true;
	}

	return false;
}


/**
 * Browse known abilities -BR-
 */
static void view_abilities(void)
{
	struct player_ability *ability;
	int num_abilities = 0;
	struct player_ability ability_list[32];

	/* Count the number of class powers we have */
	for (ability = player_abilities; ability; ability = ability->next) {
		if (class_has_ability(player->class, ability)) {
			memcpy(&ability_list[num_abilities], ability,
				   sizeof(struct player_ability));
			ability_list[num_abilities++].group = PLAYER_FLAG_CLASS;
		}
	}

	/* Count the number of race powers we have */
	for (ability = player_abilities; ability; ability = ability->next) {
		if (race_has_ability(player->race, ability)) {
			memcpy(&ability_list[num_abilities], ability,
				   sizeof(struct player_ability));
			ability_list[num_abilities++].group = PLAYER_FLAG_RACE;
		}
	}

	/* View choices until user exits */
	view_ability_menu(ability_list, num_abilities);

	return;
}


/**
 * Interact with abilities -BR-
 */
void do_cmd_abilities(void)
{
	/* View existing abilities */
	view_abilities();

	return;
}
