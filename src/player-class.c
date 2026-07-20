/**
 * \file player-class.c
 * \brief Player classes
 *
 * Copyright (c) 2011 elly+angband@leptoquark.net. See COPYING.
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


#include "player.h"
#include "z-util.h"

struct player_class *player_id2class(guid id)
{
	struct player_class *c;
	for (c = classes; c; c = c->next)
		if (guid_eq(c->cidx, id))
			break;
	return c;
}

const char *player_class_level_30_milestone(const struct player_class *c)
{
	if (!c) return NULL;

	if (streq(c->name, "Warrior")) return "Relentless";
	if (streq(c->name, "Mage")) return "Signature Spell";
	if (streq(c->name, "Druid")) return "Greater Charms";
	if (streq(c->name, "Priest")) return "Guided Prayer";
	if (streq(c->name, "Necromancer")) return "Soul Harvest";
	if (streq(c->name, "Paladin")) return "Battle Prayer";
	if (streq(c->name, "Rogue")) return "Shadowstep";
	if (streq(c->name, "Ranger")) return "Mark Quarry";
	if (streq(c->name, "Mystagogue")) return "The Platonic Forms";
	if (streq(c->name, "Stygian Warrior")) return "Innate Bloodlust";

	return NULL;
}
