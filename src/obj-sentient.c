/**
 * \file obj-sentient.c
 * \brief functions to deal with sentient object personalities
 */

#include "angband.h"
#include "effects.h"
#include "init.h"
#include "obj-sentient.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "player-util.h"

struct sentient *sentients;

int lookup_sentient(const char *name)
{
	int i;

	for (i = 1; i < z_info->sentient_max; i++) {
		struct sentient *sentient = &sentients[i];
		if (sentient->name && streq(name, sentient->name))
			return i;
	}
	return 0;
}

const struct sentient_event *sentient_event_by_index(const struct sentient *s,
	int event_idx)
{
	const struct sentient_event *event = s->events;

	while (event && event_idx) {
		event = event->next;
		event_idx--;
	}
	return event;
}

void free_object_sentient(struct object *obj)
{
	if (!obj || !obj->sentient) return;

	mem_free(obj->sentient->timeouts);
	mem_free(obj->sentient);
	obj->sentient = NULL;
}

bool sentients_are_equal(const struct object *obj1, const struct object *obj2)
{
	if (!obj1->sentient && !obj2->sentient) return true;
	if (obj1->sentient && !obj2->sentient) return false;
	if (!obj1->sentient && obj2->sentient) return false;

	return obj1->sentient->index == obj2->sentient->index &&
		obj1->sentient->spell_fail == obj2->sentient->spell_fail &&
		obj1->sentient->spell_mana == obj2->sentient->spell_mana &&
		obj1->sentient->spell_power == obj2->sentient->spell_power;
}

int16_t modify_weight_for_sentient(int i, int16_t weight)
{
	const struct object *sentient_obj;
	int16_t result;

	assert(i >= 0 && i < z_info->sentient_max);
	sentient_obj = sentients[i].obj;

	if (of_has(sentient_obj->flags, OF_MULTIPLY_WEIGHT)) {
		int32_t scaled, q;

		assert(sentient_obj->weight >= 0);
		scaled = sentient_obj->weight > 100 ? MAX(weight, 1) : MAX(weight, 0);
		scaled *= sentient_obj->weight;
		q = scaled / 100;
		if (q < 32767) {
			result = q;
			if (scaled % 100 >= 50) {
				++result;
			}
		} else {
			result = 32767;
		}
	} else {
		weight = MAX(0, weight);
		if (sentient_obj->weight < 0) {
			result = weight + sentient_obj->weight;
			if (result < 0) {
				result = 0;
			}
		} else {
			result = (weight < 32767 - sentient_obj->weight) ?
				weight + sentient_obj->weight : 32767;
		}
	}

	return result;
}

static void apply_sentient_attributes(struct object *obj, int pick)
{
	struct object *sentient_obj = sentients[pick].obj;
	int i;

	obj->weight = modify_weight_for_sentient(pick, obj->weight);
	obj->ac = add_guardi16(obj->ac, sentient_obj->ac);
	obj->to_a = add_guardi16(obj->to_a, sentient_obj->to_a);
	obj->to_h = add_guardi16(obj->to_h, sentient_obj->to_h);
	obj->to_d = add_guardi16(obj->to_d, sentient_obj->to_d);

	of_union(obj->flags, sentient_obj->flags);
	for (i = 0; i < OBJ_MOD_MAX; i++) {
		obj->modifiers[i] = add_guardi16(obj->modifiers[i],
			sentient_obj->modifiers[i]);
	}
	for (i = 0; i < ELEM_MAX; i++) {
		if (sentient_obj->el_info[i].res_level) {
			obj->el_info[i].res_level = sentient_obj->el_info[i].res_level;
		}
		obj->el_info[i].flags |= sentient_obj->el_info[i].flags;
	}
}

bool append_object_sentient(struct object *obj, int pick)
{
	struct sentient *s = &sentients[pick];
	struct sentient_event *event;
	int event_idx = 0;

	if (!obj || pick <= 0 || pick >= z_info->sentient_max) return false;
	if (obj->curses || obj->sentient) return false;
	if (!s->poss[obj->tval]) return false;

	obj->sentient = mem_zalloc(sizeof(*obj->sentient));
	obj->sentient->index = pick;
	obj->sentient->spell_fail = randcalc(s->spell_fail, 0, RANDOMISE);
	obj->sentient->spell_mana = randcalc(s->spell_mana, 0, RANDOMISE);
	obj->sentient->spell_power = randcalc(s->spell_power, 0, RANDOMISE);
	if (!obj->sentient->spell_mana) obj->sentient->spell_mana = 100;
	if (!obj->sentient->spell_power) obj->sentient->spell_power = 100;
	if (s->event_count) {
		obj->sentient->timeouts = mem_zalloc(s->event_count *
			sizeof(*obj->sentient->timeouts));
	}

	for (event = s->events; event; event = event->next) {
		obj->sentient->timeouts[event_idx++] =
			randcalc(event->time, 0, RANDOMISE);
	}

	apply_sentient_attributes(obj, pick);
	return true;
}

bool do_sentient_effect(int event_idx, struct object *obj)
{
	struct sentient *s;
	const struct sentient_event *event;
	bool ident = false;

	if (!obj || !obj->sentient) return false;
	s = &sentients[obj->sentient->index];
	event = sentient_event_by_index(s, event_idx);
	if (!event || !event->effect) return false;

	if (event->effect_msg) {
		msgt(MSG_GENERIC, "%s", event->effect_msg);
	}
	effect_do(event->effect, source_trap(NULL), NULL, &ident, true, 0, 0, 0,
		NULL);
	disturb(player);
	return ident;
}

static bool sentient_book_has_magic(const struct object *book)
{
	return book && book->sentient && tval_is_book(book);
}

int sentient_book_fail_adjust(const struct object *book)
{
	if (!sentient_book_has_magic(book)) return 0;
	return book->sentient->spell_fail;
}

int sentient_book_mana_cost(const struct object *book, int base_mana)
{
	int pct, mana;

	if (!sentient_book_has_magic(book)) return base_mana;
	pct = book->sentient->spell_mana ? book->sentient->spell_mana : 100;
	pct = MAX(0, pct);
	mana = (base_mana * pct + 50) / 100;
	if (base_mana > 0 && pct > 0 && mana < 1) mana = 1;
	return mana;
}

int sentient_book_power_percent(const struct object *book)
{
	if (!sentient_book_has_magic(book)) return 100;
	return book->sentient->spell_power ? book->sentient->spell_power : 100;
}

int sentient_book_value_bonus(const struct object *book)
{
	int fail, mana, power, value;

	if (!sentient_book_has_magic(book)) return 0;
	fail = book->sentient->spell_fail;
	mana = book->sentient->spell_mana ? book->sentient->spell_mana : 100;
	power = book->sentient->spell_power ? book->sentient->spell_power : 100;

	value = 500;
	value += (power - 100) * 100;
	value += (100 - mana) * 60;
	value -= fail * 80;
	if (value < 250) value = 250;
	return value;
}
