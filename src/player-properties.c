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
#include "player-spell.h"
#include "player-timed.h"
#include "player-util.h"
#include "project.h"
#include "target.h"
#include "ui-target.h"

typedef void (*player_power_handler)(int dir, struct command *cmd);

struct player_power {
	int flag;
	enum player_power_source source;
	int level;
	const char *name;
	bool needs_direction;
	player_power_handler handler;
};

static void use_steal(int dir, struct command *cmd);
static void use_shadowstep(int dir, struct command *cmd);
static void use_mark_quarry(int dir, struct command *cmd);
static void use_mana_siphon(int dir, struct command *cmd);
static void use_prayer(int dir, struct command *cmd);
static void use_battle_prayer(int dir, struct command *cmd);
static void use_signature_spell(int dir, struct command *cmd);
static void use_innate_bloodlust(int dir, struct command *cmd);
static void use_rage(int dir, struct command *cmd);
static void use_kobold_scurry(int dir, struct command *cmd);
static void use_siren_song(int dir, struct command *cmd);
static void use_war_cry(int dir, struct command *cmd);
static void use_satyr_trickery(int dir, struct command *cmd);
static void use_stone_lore(int dir, struct command *cmd);
static void use_demigod_taunt(int dir, struct command *cmd);
static void use_fletch_ammo(int dir, struct command *cmd);

static const struct player_power player_powers[] = {
	{ PF_STEAL, PLAYER_POWER_CLASS, 1, "Steal", true, use_steal },
	{ PF_SHADOWSTEP, PLAYER_POWER_CLASS, 30, "Shadowstep", false,
		use_shadowstep },
	{ PF_MARK_QUARRY, PLAYER_POWER_CLASS, 30, "Mark Quarry", false,
		use_mark_quarry },
	{ PF_MANA_STEAL, PLAYER_POWER_CLASS, 1, "Siphon Mana", true,
		use_mana_siphon },
	{ PF_PRAYER, PLAYER_POWER_CLASS, 1, "Prayer", false, use_prayer },
	{ PF_BATTLE_PRAYER, PLAYER_POWER_CLASS, 30, "Battle Prayer", false,
		use_battle_prayer },
	{ PF_SIGNATURE_SPELL, PLAYER_POWER_CLASS, 30, "Signature Spell", false,
		use_signature_spell },
	{ PF_INNATE_BLOODLUST, PLAYER_POWER_CLASS, 30, "Bloodlust", false,
		use_innate_bloodlust },
	{ PF_RAGE, PLAYER_POWER_RACE, 1, "Rage", false,
		use_rage },
	{ PF_KOBOLD_SCURRY, PLAYER_POWER_RACE, 1, "Scurry", false,
		use_kobold_scurry },
	{ PF_SIREN_SONG, PLAYER_POWER_RACE, 1, "Siren Song", false,
		use_siren_song },
	{ PF_WAR_CRY, PLAYER_POWER_RACE, 1, "War Cry", false, use_war_cry },
	{ PF_SATYR_TRICKERY, PLAYER_POWER_RACE, 1, "Pan's Door", false,
		use_satyr_trickery },
	{ PF_STONE_LORE, PLAYER_POWER_RACE, 1, "Stone Lore", false,
		use_stone_lore },
	{ PF_DEMIGOD_TAUNT, PLAYER_POWER_RACE, 1, "Taunt", false,
		use_demigod_taunt },
	{ PF_FLETCH_AMMO, PLAYER_POWER_RACE, 1, "Fletch Ammo", true,
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

enum battle_prayer_boon {
	BATTLE_PRAYER_SMITE,
	BATTLE_PRAYER_REBUKE,
	BATTLE_PRAYER_CONSECRATE,
	BATTLE_PRAYER_WARD,
	BATTLE_PRAYER_BANE
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

static bool player_can_use_power(const struct player_power *power)
{
	return player_owns_power(power) && player->lev >= power->level;
}

static const struct player_power *find_player_power(
		enum player_power_source source)
{
	size_t i;

	for (i = 0; i < N_ELEMENTS(player_powers); i++) {
		if (player_powers[i].source == source &&
				player_can_use_power(&player_powers[i])) {
			return &player_powers[i];
		}
	}

	return NULL;
}

static int player_power_choices(enum player_power_source source,
		const struct player_power **choices)
{
	int count = 0;
	size_t i;

	for (i = 0; i < N_ELEMENTS(player_powers); i++) {
		if (player_powers[i].source == source &&
				player_can_use_power(&player_powers[i])) {
			choices[count++] = &player_powers[i];
		}
	}

	return count;
}

const char *player_power_name(enum player_power_source source)
{
	const struct player_power *choices[N_ELEMENTS(player_powers)];
	int count = player_power_choices(source, choices);

	if (count > 1) {
		return source == PLAYER_POWER_CLASS ? "Class Skill" :
			"Racial Expertise";
	}

	return count ? choices[0]->name : NULL;
}

bool player_power_needs_direction(enum player_power_source source)
{
	const struct player_power *power = find_player_power(source);

	return power && power->needs_direction;
}

void use_player_power(enum player_power_source source, struct command *cmd)
{
	const struct player_power *choices[N_ELEMENTS(player_powers)];
	const struct player_power *power = NULL;
	const char *names[N_ELEMENTS(player_powers)];
	int count = player_power_choices(source, choices);
	int dir = 0;
	int i;

	if (player->timed[TMD_NAUSEATED]) {
		msg("You are too nauseated to focus!");
		return;
	}

	if (!count) {
		msg(source == PLAYER_POWER_CLASS ?
			"You have no class skill to use." :
			"You have no racial expertise to use.");
		return;
	}

	if (count == 1) {
		power = choices[0];
	} else {
		int choice;

		for (i = 0; i < count; i++) {
			names[i] = choices[i]->name;
		}

		choice = get_power_menu(source == PLAYER_POWER_CLASS ?
			"Use which class skill? " : "Use which racial expertise? ",
			names, count);
		if (choice < 0 || choice >= count) {
			return;
		}

		power = choices[choice];
	}

	if (power->needs_direction &&
			cmd_get_direction(cmd, "direction", &dir, false) != CMD_OK) {
		return;
	}

	power->handler(dir, cmd);
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

static void use_steal(int dir, struct command *cmd)
{
	struct monster *mon = power_target_monster(&dir);
	(void)cmd;

	if (mon) {
		steal_monster_item(mon, -1);
	} else {
		msg("You spin around.");
	}
}

static void use_shadowstep(int dir, struct command *cmd)
{
	bool ident = false;
	int duration = damroll(10, 2);
	(void)dir;
	(void)cmd;

	if (player->timed[TMD_SHADOWSTEP_COOLDOWN]) {
		msg("You need %d more turns before another shadowstep.",
			player->timed[TMD_SHADOWSTEP_COOLDOWN]);
		return;
	}

	player->upkeep->energy_use = z_info->move_energy;
	msg("You slip through a fold of shadow.");
	effect_simple(EF_TELEPORT, source_player(), "5", 0, 0, 0, 0, 0,
		&ident);
	(void)player_inc_timed(player, TMD_COVERTRACKS, duration, true, false,
		false);
	(void)player_set_timed(player, TMD_SHADOWSTEP_COOLDOWN, 100, true,
		false);
}

static void use_mark_quarry(int dir, struct command *cmd)
{
	struct monster *mon;
	char m_name[80];
	(void)dir;
	(void)cmd;

	if (player->marked_quarry[0]) {
		msg("You have already marked %s as your quarry.",
			player->marked_quarry);
		return;
	}

	msg("Choose a quarry to mark.");
	if (!target_set_interactive(TARGET_KILL, -1, -1, false)) {
		return;
	}

	mon = target_get_monster();
	if (!mon || !target_able(mon) || !mon->race || !mon->race->base) {
		msg("You have not chosen a worthy quarry.");
		return;
	}

	monster_desc(m_name, sizeof(m_name), mon, MDESC_TARG);
	my_strcpy(player->marked_quarry, mon->race->base->name,
		sizeof(player->marked_quarry));

	player->upkeep->energy_use = z_info->move_energy;
	msg("You study %s, committing its kind to the long memory of the hunt.",
		m_name);
}

static void use_mana_siphon(int dir, struct command *cmd)
{
	struct monster *mon = power_target_monster(&dir);
	char m_name[80];
	int mlevel, spell_power, chance, roll;
	int missing_mana, mana;
	(void)cmd;

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

static void use_prayer(int dir, struct command *cmd)
{
	enum prayer_boon boon;
	int choices[N_ELEMENTS(prayer_boons)];
	int i;
	(void)dir;
	(void)cmd;

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

static bool battle_prayer_cure(void)
{
	bool changed = false;

	changed |= player_clear_timed(player, TMD_AFRAID, true, false);
	changed |= player_dec_timed(player, TMD_CUT, 30, true, false);
	changed |= player_dec_timed(player, TMD_STUN, 20, true, false);

	return changed;
}

static void battle_prayer_apply(enum battle_prayer_boon boon)
{
	int wis = prayer_wis_bonus();
	int duration, dam;
	bool ident = false;

	switch (boon) {
		case BATTLE_PRAYER_SMITE:
			dam = 40 + player->lev * 2 + wis * 2;
			msg("A holy force smites the wicked around you.");
			effect_simple(EF_PROJECT_LOS_AWARE, source_player(),
				format("%d", dam), PROJ_DISP_EVIL, 0, 0, 0, 0, &ident);
			break;

		case BATTLE_PRAYER_REBUKE:
			dam = 40 + player->lev + wis * 2;
			msg("Your prayer rebukes evil.");
			effect_simple(EF_PROJECT_LOS, source_player(), format("%d", dam),
				PROJ_TURN_EVIL, 0, 0, 0, 0, &ident);
			break;

		case BATTLE_PRAYER_CONSECRATE:
			duration = 20 + player->lev / 2 + wis;
			msg("Your weapon shines with sacred purpose.");
			(void)player_inc_timed(player, TMD_ATT_EVIL, duration, true,
				false, false);
			break;

		case BATTLE_PRAYER_WARD:
			duration = 25 + player->lev + wis;
			msg("A warding light surrounds you.");
			(void)player_inc_timed(player, TMD_PROTEVIL, duration, true,
				false, false);
			break;

		case BATTLE_PRAYER_BANE:
			duration = 20 + player->lev / 2 + wis;
			msg("Your vow burns against demons.");
			(void)player_inc_timed(player, TMD_ATT_DEMON, duration, true,
				false, false);
			break;
	}
}

static void use_battle_prayer(int dir, struct command *cmd)
{
	enum battle_prayer_boon boon = randint0(5);
	int wis = prayer_wis_bonus();
	int heal = 30 + player->lev * 2 + wis * 2;
	bool ident = false;
	(void)dir;
	(void)cmd;

	if (player->lev < 30) {
		msg("You must reach level 30 to offer a battle prayer.");
		return;
	}

	if (player->timed[TMD_BATTLE_PRAYER_COOLDOWN]) {
		msg("You need %d more turns before another battle prayer.",
			player->timed[TMD_BATTLE_PRAYER_COOLDOWN]);
		return;
	}

	player->upkeep->energy_use = z_info->move_energy;
	msg("You offer a battle prayer.");

	effect_simple(EF_HEAL_HP, source_player(), format("%d", heal),
		0, 0, 0, 0, 0, &ident);
	(void)battle_prayer_cure();
	battle_prayer_apply(boon);
	(void)player_set_timed(player, TMD_BATTLE_PRAYER_COOLDOWN, 240, true,
		false);
}

static int signature_spell_cooldown(const struct class_spell *spell)
{
	return 30 + spell->slevel * 4 + spell->smana * 3;
}

static bool signature_spell_is_learned(int spell_index)
{
	const struct class_spell *spell = spell_by_index(player, spell_index);

	return spell && spell->slevel <= player->lev &&
		(player->spell_flags[spell_index] & PY_SPELL_LEARNED) &&
		!(player->spell_flags[spell_index] & PY_SPELL_FORGOTTEN);
}

static bool choose_signature_spell(void)
{
	int total_spells = player->class->magic.total_spells;
	const char **choices = mem_zalloc(total_spells * sizeof(*choices));
	char (*entries)[160] = mem_zalloc(total_spells * sizeof(*entries));
	int *choice_map = mem_zalloc(total_spells * sizeof(*choice_map));
	int i, count = 0, choice;

	for (i = 0; i < total_spells; i++) {
		const struct class_spell *spell;
		char info[80];

		if (!signature_spell_is_learned(i)) continue;

		spell = spell_by_index(player, i);
		get_spell_info(i, info, sizeof(info));
		strnfmt(entries[count], sizeof(entries[count]),
			"%-24s Lv %2d  Rchg %3d  Fail %2d%%  %s",
			spell->name, spell->slevel, signature_spell_cooldown(spell),
			spell_chance_free(i), info);
		choices[count] = entries[count];
		choice_map[count++] = i;
	}

	if (!count) {
		msg("You do not know any spells to bind.");
		mem_free(choice_map);
		mem_free(entries);
		mem_free(choices);
		return false;
	}

	choice = get_power_menu("Choose your signature spell: ", choices, count);
	if (choice >= 0 && choice < count) {
		player->signature_spell = choice_map[choice];
		msg("You bind the spell to the architecture of your mind.");
	}

	mem_free(choice_map);
	mem_free(entries);
	mem_free(choices);

	return choice >= 0 && choice < count;
}

static void use_signature_spell(int dir, struct command *cmd)
{
	const struct class_spell *spell;
	int cooldown;

	(void)dir;

	if (player->lev < 30) {
		msg("You must reach level 30 to bind a signature spell.");
		return;
	}

	if (player->timed[TMD_SIGNATURE_SPELL_COOLDOWN]) {
		msg("Your mind needs %d more turns to re-form the pattern.",
			player->timed[TMD_SIGNATURE_SPELL_COOLDOWN]);
		return;
	}

	if (player->signature_spell < 0) {
		(void)choose_signature_spell();
		return;
	}

	spell = spell_by_index(player, player->signature_spell);
	if (!spell) {
		player->signature_spell = -1;
		msg("Your signature spell has slipped from memory.");
		return;
	}

	if (!player_can_cast(player, true)) {
		return;
	}

	if (spell_needs_aim(player->signature_spell)) {
		if (cmd_get_target(cmd, "target", &dir) == CMD_OK) {
			player_confuse_dir(player, &dir, false);
		} else {
			return;
		}
	}

	target_fix();
	msg("You release your signature spell.");
	if (spell_cast_free(player->signature_spell, dir, cmd)) {
		if (player->timed[TMD_FASTCAST]) {
			player->upkeep->energy_use = (z_info->move_energy * 3) / 4;
		} else {
			player->upkeep->energy_use = z_info->move_energy;
		}
		cooldown = signature_spell_cooldown(spell);
		(void)player_set_timed(player, TMD_SIGNATURE_SPELL_COOLDOWN,
			cooldown, true, false);
	}
	target_release();
}

static void use_innate_bloodlust(int dir, struct command *cmd)
{
	(void)dir;
	(void)cmd;

	if (player->lev < 30) {
		msg("You must reach level 30 to call up bloodlust.");
		return;
	}

	if (player->timed[TMD_BLOODLUST]) {
		msg("The red mist already rules you.");
		return;
	}

	if (player->timed[TMD_INNATE_BLOODLUST_COOLDOWN]) {
		msg("You need %d more turns before calling up bloodlust again.",
			player->timed[TMD_INNATE_BLOODLUST_COOLDOWN]);
		return;
	}

	player->upkeep->energy_use = z_info->move_energy;
	msg("You call the red mist into your blood.");
	(void)player_set_timed(player, TMD_BLOODLUST, 10, true, false);
	(void)player_set_timed(player, TMD_INNATE_BLOODLUST_COOLDOWN, 360,
		true, false);
}

static void use_rage(int dir, struct command *cmd)
{
	int duration;
	(void)dir;
	(void)cmd;

	if (player->timed[TMD_RAGE] || player->timed[TMD_SHERO]) {
		msg("You are already consumed by rage.");
		return;
	}

	if (player->timed[TMD_RAGE_COOLDOWN]) {
		msg("You need %d more turns before unleashing your rage again.",
			player->timed[TMD_RAGE_COOLDOWN]);
		return;
	}

	player->upkeep->energy_use = z_info->move_energy;
	(void)player_clear_timed(player, TMD_AFRAID, true, false);

	duration = 10 + randint1(10) + player->lev / 2;
	(void)player_inc_timed(player, TMD_RAGE, duration, true,
		false, false);
	(void)player_inc_timed(player, TMD_RUNNING, duration, true, false, false);
	(void)player_set_timed(player, TMD_RAGE_COOLDOWN, 150, true,
		false);
	monsters_handle_player_noise(100);
}

static void use_kobold_scurry(int dir, struct command *cmd)
{
	(void)dir;
	(void)cmd;

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

static void use_siren_song(int dir, struct command *cmd)
{
	const struct siren_song *song;
	const char *choices[N_ELEMENTS(siren_songs)];
	char entries[N_ELEMENTS(siren_songs)][96];
	int choice_map[N_ELEMENTS(siren_songs)];
	struct monster *mon;
	char m_name[80];
	int choice, chance, duration, count = 0;
	size_t i;
	(void)cmd;

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

static void use_war_cry(int dir, struct command *cmd)
{
	struct monster *mon;
	char m_name[80];
	(void)cmd;

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

static void use_satyr_trickery(int dir, struct command *cmd)
{
	bool ident = false;
	(void)dir;
	(void)cmd;

	if (player->timed[TMD_SATYR_TRICKERY_COOLDOWN]) {
		msg("You need %d more turns before opening Pan's Door again.",
			player->timed[TMD_SATYR_TRICKERY_COOLDOWN]);
		return;
	}

	player->upkeep->energy_use = z_info->move_energy;
	msg("A small portal opens before you.");
	effect_simple(EF_TELEPORT, source_player(), "8", 0, 0, 0, 0, 0,
		&ident);
	(void)player_set_timed(player, TMD_SATYR_TRICKERY_COOLDOWN, 80,
		true, false);
}

static void use_stone_lore(int dir, struct command *cmd)
{
	const char *choices[N_ELEMENTS(stone_lore)];
	char entries[N_ELEMENTS(stone_lore)][96];
	int choice_map[N_ELEMENTS(stone_lore)];
	const struct stone_lore *lore;
	int choice, count = 0;
	size_t i;
	bool ident = false;
	(void)dir;
	(void)cmd;

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

static void use_demigod_taunt(int dir, struct command *cmd)
{
	(void)dir;
	(void)cmd;

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

static void use_fletch_ammo(int dir, struct command *cmd)
{
	const char *choices[] = { "Arrows", "Bolts" };
	int choice, tval, sval, quantity;
	struct object_kind *kind;
	struct object *ammo;
	struct loc grid;
	int feat;
	char o_name[80];
	(void)cmd;

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
