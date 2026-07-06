/**
 * \file obj-sentient.h
 * \brief functions to deal with sentient object personalities
 */

#ifndef OBJECT_SENTIENT_H_
#define OBJECT_SENTIENT_H_

extern struct sentient *sentients;

int lookup_sentient(const char *name);
bool sentients_are_equal(const struct object *obj1, const struct object *obj2);
bool append_object_sentient(struct object *obj, int pick);
void free_object_sentient(struct object *obj);
bool do_sentient_effect(int event_idx, struct object *obj);
const struct sentient_event *sentient_event_by_index(const struct sentient *s,
	int event_idx);
int16_t modify_weight_for_sentient(int i, int16_t weight);

#endif /* OBJECT_SENTIENT_H_ */
