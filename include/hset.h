#ifndef HSET_H
#define HSET_H

#include "../include/hash.h"

#define HSET_FOREACH_ENTRY(ENTRY, HSET, START, END) \
   for (unsigned int i = 0; (ENTRY = set_iterate_r(HSET, START)) && i < END ; i++)

typedef dictp setp;

static inline setp set_new(size_t key_sz) {
    return dict_new2(key_sz, 0);
}

//TODO: make this function return true/false
static inline setp set_put(setp set, keyp key) {
    int val = 0;
    if (dict_get(set, key) == NULL)
        dict_put(set, key, &val);
    return set;
}

static inline bool set_in(setp set, keyp memb) {
    return dict_get(set, memb) != NULL;
}

static inline setp set_config_va(setp set, va_list vargs) {
    return dict_config_va(set, vargs);
}

static inline setp set_config(setp set, ...) {
    va_list vargs;
    va_start(vargs, set);
    set_config_va(set, vargs);
    va_end(vargs);
    return set;
}

static inline keyp set_iterate_r(setp set, ulong *state) {
    return dict_iterate_r(set, state);
}

static inline keyp set_iterate(setp set) {
    return dict_iterate(set);
}

setp set_cpy(setp S);

setp set_union(setp A, setp B);

setp set_union_inplace(setp A, setp B);

void set_free(setp S);

#endif
