#include "../include/hash.h"

uint djb2(keyp key, size_t key_sz) {
    /* djb2 hashing function from http://www.cse.yorku.ca/~oz/hash.html */
    uint hash = 5381;
    for (uint i = 0; i < key_sz; i++) {
        hash = ((hash << 5) + hash) + ((uint8_t *) key)[i];
    }
    return hash;
}

uint djb2_str(keyp key, size_t key_sz) {
    /* djb2 hashing function from http://www.cse.yorku.ca/~oz/hash.html */
    uint hash = 5381;
    uint i = 0;
    for (; i < key_sz && ((char *) key)[i]; i++) {
        hash = ((hash << 5) + hash) + ((char *) key)[i];
    }
    return hash;
}

void htab_init(htab_t *ht, ht_hash_func h, size_t key_sz, size_t val_sz,
               ulong buf_cap) {
    ht->key_sz = key_sz;
    ht->val_sz = val_sz;
    ht->buf_cap = buf_cap;
    ht->buf_load = 0;
    ht->h = h;
    ht->cmp = (ht_cmp_func) memcmp;
    ht->keycpy = (ht_key_cpy_func) memcpy;
    memset(ht->buf, 0, buf_cap * htab_entry_size(ht));
}

htab_t *htab_new(ht_hash_func h, size_t key_sz, size_t val_sz, ulong buf_cap) {
    htab_t *new =
            malloc(sizeof(htab_t) + htab_entry_size2(key_sz, val_sz) * buf_cap);
    htab_init(new, h, key_sz, val_sz, buf_cap);
    return new;
}

void htab_destroy(htab_t *ht, void (*free_t)(void *)) {
    htab_free_entries(ht, free_t);
    free(ht);
}

void htab_free_entries(htab_t *ht, void (*free_t)(void *)) {
    htab_entry_t *bucket;
    size_t entry_sz = htab_entry_size(ht);
    for (uint i = 0; i < ht->buf_cap; i++) {
        bucket = (void *) &ht->buf[entry_sz * i];
        if (bucket->flags & HT_ENTRY_FLAGS_OCCUPIED)
            free_t(bucket->contents + ht->key_sz);
    }
}

bool htab_put(htab_t *ht, keyp key, valp val) {
    uint hash = ht->h(key, ht->key_sz);
    size_t entry_sz = htab_entry_size(ht);
    htab_entry_t *bucket = (void *) &ht->buf[entry_sz * (hash % ht->buf_cap)];
    for (int probes = 0; probes < MAX_PROBES; probes++) {
        if (!bucket->flags || (bucket->flags & HT_ENTRY_FLAGS_DELETED)) {
            /* bucket is empty; add the entry */
            bucket->flags = HT_ENTRY_FLAGS_OCCUPIED;
            ht->buf_load++; /* increase the load */
            bucket->hash = hash;
            ht->keycpy(bucket->contents, key, ht->key_sz);
            memcpy(bucket->contents + ht->key_sz, val, ht->val_sz);
            return true;
        }
        /* pseudo-random probing */
        bucket = (void *) &ht->buf[entry_sz * (rand_r(&hash) % ht->buf_cap)];
    }
    /* we failed to put the element in the hash table */
    return false;
}

valp htab_get(htab_t *ht, keyp key) {
    uint hash = ht->h(key, ht->key_sz);
    size_t entry_sz = htab_entry_size(ht);

    htab_entry_t *bucket = (void *) &ht->buf[entry_sz * (hash % ht->buf_cap)];

    while (bucket->flags) {
        /* while the bucket is not empty; */
        if (!(bucket->flags & HT_ENTRY_FLAGS_DELETED) && hash == bucket->hash &&
            (ht->cmp(bucket->contents, key, ht->key_sz) == 0)) {
            /* we found it! */
            return bucket->contents + ht->key_sz;
        }
        bucket = (void *) &ht->buf[entry_sz * (rand_r(&hash) % ht->buf_cap)];
    }

    /* we failed to put the element in the hash table */
    return NULL;
}

const keyp htab_get_keyp_from_valp(htab_t *ht, valp val) {
    return val - ht->key_sz;
}

const keyp htab_get_keyp(htab_t *ht_info, keyp key) {
    valp val = htab_get(ht_info, key);
    if (val != NULL) {
        return htab_get_keyp_from_valp(ht_info, val);
    } else {
        return NULL;
    }
}

valp htab_del(htab_t *ht, keyp key) {
    valp val = htab_get(ht, key);
    if (val == NULL)
        /* key was not in the hash table */
        return NULL;

    htab_entry_t *bucket = val - offsetof(htab_entry_t, contents) - ht->key_sz;
    bucket->flags = HT_ENTRY_FLAGS_DELETED;
    ht->buf_load--;
    return val;
}

bool htab_rehash(htab_t *old, htab_t *new) {
    size_t entsz_old = htab_entry_size(old);
    char *new_key = malloc(new->key_sz);
    for (uint i = 0; i < old->buf_cap; i++) {
        htab_entry_t *bucket = (void *) &old->buf[entsz_old * i];
        if (bucket->flags & HT_ENTRY_FLAGS_OCCUPIED) {
            /* bucket is occupied */
            old->keycpy(new_key, bucket->contents, old->key_sz);
            htab_put(new, new_key, bucket->contents + old->key_sz);
        }
    }
    free(new_key);
    return true;
}

bool htab_rehash_deep(htab_t *old, htab_t *new, valp (*copy)(valp)) {
    size_t entsz = htab_entry_size(old);
    for (uint i = 0; i < old->buf_cap; i++) {
        htab_entry_t *bucket = (void *) &old->buf[entsz * i];
        if (bucket->flags & HT_ENTRY_FLAGS_OCCUPIED) {
            /* bucket is occupied */
            htab_put(new, bucket->contents,
                     copy(bucket->contents + old->key_sz));
        }
    }
    return true;
}

void *htab_iterate_r(htab_t *ht, ulong *state) {
    size_t entsz = htab_entry_size(ht);
    htab_entry_t *bucket = (void *) &ht->buf[*state * entsz];
    while (*state < ht->buf_cap && bucket->flags != HT_ENTRY_FLAGS_OCCUPIED) {
        (*state)++;
        bucket = (void *) &ht->buf[*state * entsz];
    }

    if (*state < ht->buf_cap) {
        (*state)++;
        return bucket->contents;
    }

    return NULL;
}

void *htab_iterate(htab_t *ht) {
    static ulong state = 0;
    static htab_t *htab = NULL;

    if (ht != htab) {
        htab = ht;
        state = 0;
    }

    return htab_iterate_r(htab, &state);
}

/* ========================= */
/*           DICT            */
/*         FUNCTIONS         */
/* ========================= */

dictp dict_new2(size_t key_sz, size_t val_sz) {
    return dict_new3(key_sz, val_sz, 16);
}

dictp dict_new3(size_t key_sz, size_t val_sz, ulong bufcap) {
    dictp new = malloc(sizeof(dict_t));
    new->max_load_factor = 0.7;
    new->key_sz = NULL;
    new->htab = htab_new(djb2, key_sz, val_sz, bufcap);
    return new;
}

dictp dict_config(dictp d, ...) {
    va_list vargs;
    va_start(vargs, d);
    dict_conf_key
            k = va_arg(vargs,
                       int);
    while (k != DICT_CONF_DONE) {
        switch (k) {
            case DICT_CONF_HASH_FUNC:
                dict_set_hfunc(d, va_arg(vargs,
                                         void*));
                break;
            case DICT_CONF_KEY_CPY:
                dict_set_keycpy(d, va_arg(vargs,
                                          void*));
                break;
            case DICT_CONF_CMP:
                dict_set_cmp(d, va_arg(vargs,
                                       void*));
                break;
            case DICT_CONF_LF:
                dict_set_max_load_factor(d, va_arg(vargs,
                                                   double));
                break;
            case DICT_CONF_KEY_SZ_F:
                dict_set_key_sz_f(d, va_arg(vargs,
                                            void*));
                break;
            default:
                break;
        }
        k = va_arg(vargs,
                   int);
    }
    return d;
}

dictp dict_set_hfunc(dictp d, ht_hash_func f) {
    d->htab->h = f;
    return d;
}

dictp dict_set_keycpy(dictp d, ht_key_cpy_func f) {
    d->htab->keycpy = f;
    return d;
}

dictp dict_set_cmp(dictp d, ht_cmp_func f) {
    d->htab->cmp = f;
    return d;
}

dictp dict_set_max_load_factor(dictp d, double lf) {
    d->max_load_factor = lf;
    return d;
}

dictp dict_set_key_sz_f(dictp d, size_t (*f)(keyp key)) {
    d->key_sz = f;
    return d;
}

dictp dict_put(dictp dict, keyp key, valp val) {
    size_t key_sz = dict->htab->key_sz;
    if (dict->key_sz)
        key_sz = dict->key_sz(key);
    bool rehash = false;
    size_t new_key_sz = dict->htab->key_sz;
    ulong new_buf_load = dict->htab->buf_cap;

    while (key_sz > new_key_sz) {
        /* rehash */
        new_key_sz = new_key_sz << 1;
        rehash = true;
    }

    if (((double) (dict->htab->buf_load + 1) / dict->htab->buf_cap) >
        dict->max_load_factor) {
        new_buf_load = new_buf_load << 1;
        rehash = true;
    }

    if (rehash) {
        dict_force_rehash3(dict, new_buf_load, new_key_sz);
    }

    while (!htab_put(dict->htab, key, val)) {
        dict_force_rehash3(dict, new_buf_load = new_buf_load << 2, new_key_sz);
    }

    return dict;
}

dictp dict_putv(dictp dict, int *num_put, ...) {
    *num_put = 0;
    va_list vargs;
    va_start(vargs, num_put);
    keyp k;
    valp v;
    do {
        k = va_arg(vargs, keyp);
        if (k != NULL) {
            v = va_arg(vargs, valp);
            dict_put(dict, k, v);
            (*num_put)++;
        }
    } while (k != NULL);
    return dict;
}

valp dict_get(dictp dict, keyp key) {
    return htab_get(dict->htab, key);
}

dictp dict_force_rehash3(dictp d, ulong new_bufcap, size_t new_keysz) {
    htab_t *new_ht =
            htab_new(d->htab->h, new_keysz, d->htab->val_sz, new_bufcap);
    new_ht->cmp = d->htab->cmp;
    new_ht->keycpy = d->htab->keycpy;
    htab_rehash(d->htab, new_ht);
    free(d->htab);
    d->htab = new_ht;
    return d;
}

dictp dict_force_rehash2(dictp d, ulong new_bufcap) {
    return dict_force_rehash3(d, new_bufcap, d->htab->key_sz);
}

valp dict_del(dictp d, keyp key) {
    return htab_del(d->htab, key);
}

keyp dict_iterate_r(dictp d, ulong *state) {
    return htab_iterate_r(d->htab, state);
}

keyp dict_iterate(dictp d) {
    return htab_iterate(d->htab);
}

void dict_free(dictp dict, void (*free_t)(void *)) {
    htab_destroy(dict->htab, free_t);
    free(dict);
}

size_t str_sz(keyp key) {
    return strlen(key) + 1;
}
