#ifndef HSET_H
#define HSET_H

#include "../include/hash.h"

typedef dictp setp;

static inline setp set_new(size_t key_sz) {
    return dict_new2(key_sz, 0);
}

static inline setp set_put(setp set, keyp key){
    int val = 0;
    return dict_put(set, key, &val);
}

static inline bool set_in(setp set, keyp memb){
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

static inline keyp set_iterate(setp set){
    return dict_iterate(set);
}

setp set_cpy(setp S);

setp set_union(setp A, setp B);
setp set_union_inplace(setp A, setp B);

void set_free(setp S);
#endif
