#include "../include/hset.h"

setp set_cpy(setp S) {
    /* clang-format off */
    /* @formatter:off */
    setp new = set_config(
            dict_new3(S->htab->key_sz, 0, S->htab->buf_cap),
            DICT_CONF_HASH_FUNC, S->htab->h,
            DICT_CONF_KEY_CPY, S->htab->keycpy,
            DICT_CONF_CMP, S->htab->cmp,
            DICT_CONF_LF, S->max_load_factor,
            DICT_CONF_KEY_SZ_F, S->key_sz
    );
    /* @formatter:on */
    /* clang-format on */

    ulong i;
    for (keyp k = set_iterate_r(S, &i);
         k != NULL;
         k = set_iterate_r(S, &i)) {
        set_put(new, k);
    }
    return new;
}

setp set_union_inplace(setp A, setp B) {
    ulong i;
    keyp k;
    for (k = set_iterate_r(B, &i);
         k != NULL;
         k = set_iterate_r(B, &i)) {
        set_put(A, k);
    }
    return A;
}

setp set_union(setp A, setp B) {
    if (A->htab->buf_load < B->htab->buf_load)
        return set_union(B, A);
    setp A_ = set_cpy(A);
    return set_union_inplace(A_, B);
}

static void nofree(void *_) {}

void set_free(setp S) {
    dict_free(S, nofree);
}
