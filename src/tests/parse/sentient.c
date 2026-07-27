/* parse/sentient */
/* Exercise parsing used for sentient.txt. */

#include "unit-test.h"
#include "datafile.h"
#include "effects.h"
#include "init.h"
#include "object.h"
#include "obj-init.h"
#include "player-timed.h"
#include "project.h"
#include "z-virt.h"

int setup_tests(void **state) {
	*state = sentient_parser.init();
	z_info = mem_zalloc(sizeof(*z_info));
	return !*state;
}

int teardown_tests(void *state) {
	struct parser *p = (struct parser*) state;
	int r = 0;

	if (sentient_parser.finish(p)) {
		r = 1;
	}
	sentient_parser.cleanup();
	mem_free(z_info);
	return r;
}

static int test_missing_record_header0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r;

	r = parser_parse(p, "alloc:50:1 to 100");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "type:boots");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "values:DEX[2] | STR[-1]");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "spell-fail:-1d4");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "spell-mana:81+1d8");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "spell-power:115+3d8");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "event:15:1d250");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "effect:DETECT_INVISIBLE_MONSTERS");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	ok;
}

static int test_name0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "name:Nimble");
	struct sentient *s;

	eq(r, PARSE_ERROR_NONE);
	s = (struct sentient*) parser_priv(p);
	notnull(s);
	require(streq(s->name, "Nimble"));
	notnull(s->obj);
	notnull(s->poss);
	null(s->events);
	eq(s->event_count, 0);
	ok;
}

static int test_alloc0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "alloc:50:1 to 100");
	struct sentient *s = (struct sentient*) parser_priv(p);

	eq(r, PARSE_ERROR_NONE);
	eq(s->alloc_prob, 50);
	eq(s->alloc_min, 1);
	eq(s->alloc_max, 100);
	ok;
}

static int test_values0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "values:DEX[2] | STR[-1]");
	struct sentient *s = (struct sentient*) parser_priv(p);

	eq(r, PARSE_ERROR_NONE);
	eq(s->obj->modifiers[OBJ_MOD_DEX], 2);
	eq(s->obj->modifiers[OBJ_MOD_STR], -1);
	ok;
}

static int test_spell_modifiers0(void *state) {
	struct parser *p = (struct parser*) state;
	struct sentient *s = (struct sentient*) parser_priv(p);
	enum parser_error r;

	r = parser_parse(p, "spell-fail:-1d4");
	eq(r, PARSE_ERROR_NONE);
	eq(s->spell_fail.base, -5);
	eq(s->spell_fail.dice, 1);
	eq(s->spell_fail.sides, 4);
	r = parser_parse(p, "spell-mana:81+1d8");
	eq(r, PARSE_ERROR_NONE);
	eq(s->spell_mana.base, 81);
	eq(s->spell_mana.dice, 1);
	eq(s->spell_mana.sides, 8);
	r = parser_parse(p, "spell-power:115+3d8");
	eq(r, PARSE_ERROR_NONE);
	eq(s->spell_power.base, 115);
	eq(s->spell_power.dice, 3);
	eq(s->spell_power.sides, 8);
	ok;
}

static int test_event_effect0(void *state) {
	struct parser *p = (struct parser*) state;
	struct sentient *s;
	enum parser_error r;

	r = parser_parse(p, "event:15:1d250");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "effect:DETECT_INVISIBLE_MONSTERS");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "msg:The Oracle reveals unseen presences.");
	eq(r, PARSE_ERROR_NONE);
	s = (struct sentient*) parser_priv(p);
	eq(s->event_count, 1);
	notnull(s->events);
	eq(s->events->chance, 15);
	notnull(s->events->effect);
	notnull(s->events->effect_msg);
	ok;
}

const char *suite_name = "parse/sentient";
struct test tests[] = {
	{ "missing_record_header0", test_missing_record_header0 },
	{ "name0", test_name0 },
	{ "alloc0", test_alloc0 },
	{ "values0", test_values0 },
	{ "spell_modifiers0", test_spell_modifiers0 },
	{ "event_effect0", test_event_effect0 },
	{ NULL, NULL }
};
