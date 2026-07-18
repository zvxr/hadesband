/* player/races */
/* Check the custom player races. */

#include "unit-test.h"
#include "test-utils.h"

#include "init.h"
#include "object.h"
#include "player.h"
#include "player-birth.h"
#include "player-calcs.h"
#include "player-properties.h"
#include "player-timed.h"

struct expected_race {
	const char *name;
	int stats[STAT_MAX];
	int hitdie;
	int exp;
	int infra;
	int melee;
	int device;
	int flag;
	int elem1;
	int elem2;
};

static const struct expected_race expected[] = {
	{
		"Cyclops",
		{ 5, -3, -2, -2, 2 },
		13, 140, 1, 20, -10,
		-1, -1, ELEM_GRAVITY
	},
	{
		"Demigod",
		{ 2, 2, 2, 2, 2 },
		10, 150, 0, 15, 5,
		OF_SUST_CON, -1, -1
	},
	{
		"Satyr",
		{ 1, -1, 2, 0, 2 },
		11, 130, 5, 9, 8,
		-1, ELEM_DISEN, -1
	},
	{
		"Siren",
		{ -3, 3, 1, 4, -2 },
		6, 125, 4, -12, 16,
		-1, -1, -1
	}
};

static struct player_race *find_race(const char *name)
{
	struct player_race *race;

	for (race = races; race; race = race->next) {
		if (streq(race->name, name)) {
			return race;
		}
	}

	return NULL;
}

static int require_race_record(const struct expected_race *e,
		struct player_race **race)
{
	int i;

	*race = find_race(e->name);
	require(*race);

	for (i = 0; i < STAT_MAX; i++) {
		eq((*race)->r_adj[i], e->stats[i]);
	}
	eq((*race)->r_mhp, e->hitdie);
	eq((*race)->r_exp, e->exp);
	eq((*race)->infra, e->infra);
	eq((*race)->r_skills[SKILL_TO_HIT_MELEE], e->melee);
	eq((*race)->r_skills[SKILL_DEVICE], e->device);

	if (e->flag >= 0) {
		require(of_has((*race)->flags, e->flag));
	}
	if (e->elem1 >= 0) {
		eq((*race)->el_info[e->elem1].res_level, 1);
	}
	if (e->elem2 >= 0) {
		eq((*race)->el_info[e->elem2].res_level, 1);
	}

	ok;
}

int setup_tests(void **state)
{
	set_file_paths();
	if (!init_angband()) {
		return 1;
	}
	return 0;
}

int teardown_tests(void *state)
{
	cleanup_angband();
	return 0;
}

static int test_records0(void *state)
{
	size_t i;

	for (i = 0; i < N_ELEMENTS(expected); i++) {
		struct player_race *race = NULL;

		eq(require_race_record(&expected[i], &race), 0);
	}

	ok;
}

static int test_bonuses0(void *data)
{
	struct player_state calc_state;
	int base_melee;

	eq(player_make_simple("Cyclops", "Warrior", "Tester"), true);
	calc_bonuses(player, &calc_state, false, false);
	eq(calc_state.el_info[ELEM_SHARD].res_level, 0);
	eq(calc_state.el_info[ELEM_GRAVITY].res_level, 1);
	require(player_has(player, PF_CYCLOPEAN_RAGE));
	base_melee = calc_state.skills[SKILL_TO_HIT_MELEE];
	player->timed[TMD_CYCLOPEAN_RAGE] = 10;
	player->timed[TMD_RUNNING] = 10;
	calc_bonuses(player, &calc_state, false, false);
	eq(calc_state.el_info[ELEM_SHARD].res_level, 1);
	eq(calc_state.skills[SKILL_TO_HIT_MELEE], base_melee + 75);
	eq(calc_state.num_moves, 1);

	eq(player_make_simple("Demigod", "Warrior", "Tester"), true);
	calc_bonuses(player, &calc_state, false, false);
	require(of_has(calc_state.flags, OF_SUST_CON));

	eq(player_make_simple("Kobold", "Warrior", "Tester"), true);
	calc_bonuses(player, &calc_state, false, false);
	eq(calc_state.el_info[ELEM_POIS].res_level, 1);
	require(player_has(player, PF_KOBOLD_SCURRY));
	eq(calc_state.num_moves, 0);
	player->timed[TMD_RUNNING] = 20;
	calc_bonuses(player, &calc_state, false, false);
	eq(calc_state.num_moves, 1);

	eq(player_make_simple("Satyr", "Warrior", "Tester"), true);
	calc_bonuses(player, &calc_state, false, false);
	eq(calc_state.el_info[ELEM_DISEN].res_level, 1);
	require(player_has(player, PF_KNOW_MUSHROOM));
	require(player_has(player, PF_CURE_CONFUSION));

	eq(player_make_simple("Siren", "Warrior", "Tester"), true);
	calc_bonuses(player, &calc_state, false, false);
	require(player_has(player, PF_SIREN_SONG));

	eq(player_make_simple("Elf", "Warrior", "Tester"), true);
	calc_bonuses(player, &calc_state, false, false);
	require(player_has(player, PF_FLETCH_AMMO));

	ok;
}

static int test_power_ownership0(void *data)
{
	struct player_class class_copy;
	const struct player_class *original_class;

	eq(player_make_simple("Cyclops", "Warrior", "Tester"), true);
	require(streq(player_power_name(PLAYER_POWER_RACE), "Cyclopean Rage"));
	require(!player_power_needs_direction(PLAYER_POWER_RACE));
	null(player_power_name(PLAYER_POWER_CLASS));

	original_class = player->class;
	memcpy(&class_copy, player->class, sizeof(class_copy));
	pf_on(class_copy.pflags, PF_STEAL);
	player->class = &class_copy;
	require(streq(player_power_name(PLAYER_POWER_CLASS), "Steal"));
	require(player_power_needs_direction(PLAYER_POWER_CLASS));
	player->class = original_class;

	eq(player_make_simple("Kobold", "Warrior", "Tester"), true);
	require(streq(player_power_name(PLAYER_POWER_RACE), "Scurry"));
	require(!player_power_needs_direction(PLAYER_POWER_RACE));

	eq(player_make_simple("Half-Orc", "Warrior", "Tester"), true);
	require(streq(player_power_name(PLAYER_POWER_RACE), "War Cry"));
	require(!player_power_needs_direction(PLAYER_POWER_RACE));

	eq(player_make_simple("Dwarf", "Warrior", "Tester"), true);
	require(streq(player_power_name(PLAYER_POWER_RACE), "Stone Lore"));
	require(!player_power_needs_direction(PLAYER_POWER_RACE));

	eq(player_make_simple("Satyr", "Warrior", "Tester"), true);
	require(streq(player_power_name(PLAYER_POWER_RACE), "Cure Confusion"));
	require(!player_power_needs_direction(PLAYER_POWER_RACE));

	eq(player_make_simple("Siren", "Warrior", "Tester"), true);
	require(streq(player_power_name(PLAYER_POWER_RACE), "Siren Song"));
	require(!player_power_needs_direction(PLAYER_POWER_RACE));

	eq(player_make_simple("Demigod", "Warrior", "Tester"), true);
	require(streq(player_power_name(PLAYER_POWER_RACE), "Taunt"));
	require(!player_power_needs_direction(PLAYER_POWER_RACE));

	eq(player_make_simple("Elf", "Warrior", "Tester"), true);
	require(streq(player_power_name(PLAYER_POWER_RACE), "Fletch Ammo"));
	require(player_power_needs_direction(PLAYER_POWER_RACE));

	eq(player_make_simple("Human", "Warrior", "Tester"), true);
	null(player_power_name(PLAYER_POWER_RACE));

	/* Derived flags must not grant a class Skill or racial Expertise. */
	pf_on(player->state.pflags, PF_CYCLOPEAN_RAGE);
	null(player_power_name(PLAYER_POWER_RACE));

	ok;
}

const char *suite_name = "player/races";
struct test tests[] = {
	{ "records0", test_records0 },
	{ "bonuses0", test_bonuses0 },
	{ "power ownership0", test_power_ownership0 },
	{ NULL, NULL }
};
