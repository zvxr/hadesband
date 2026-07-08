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
#include "game-input.h"
#include "init.h"
#include "mon-desc.h"
#include "mon-move.h"
#include "mon-predicate.h"
#include "mon-spell.h"
#include "mon-util.h"
#include "player-timed.h"
#include "player-util.h"

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
static void use_cyclopean_rage(int dir);

static const struct player_power player_powers[] = {
	{ PF_STEAL, PLAYER_POWER_CLASS, "Steal", true, use_steal },
	{ PF_MANA_STEAL, PLAYER_POWER_CLASS, "Siphon Mana", true,
		use_mana_siphon },
	{ PF_CYCLOPEAN_RAGE, PLAYER_POWER_RACE, "Cyclopean Rage", false,
		use_cyclopean_rage }
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

static void use_cyclopean_rage(int dir)
{
	int cost;
	int duration;

	if (player->timed[TMD_CYCLOPEAN_RAGE] || player->timed[TMD_SHERO]) {
		msg("You are already consumed by rage.");
		return;
	}

	cost = MAX(1, player->mhp / 20);
	if (player->chp <= cost) {
		msg("You are too weak to unleash your Cyclopean rage.");
		return;
	}

	player->upkeep->energy_use = z_info->move_energy;
	take_hit(player, cost, "the strain of Cyclopean rage");
	(void)player_clear_timed(player, TMD_AFRAID, true, false);

	duration = 10 + randint1(10) + player->lev / 2;
	(void)player_inc_timed(player, TMD_CYCLOPEAN_RAGE, duration, true,
		false, false);
	monsters_handle_player_noise(100);
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
