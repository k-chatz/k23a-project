#include "../include/hash.h"

uint djb2(keyp key, size_t key_sz){
    /* djb2 hashing function from http://www.cse.yorku.ca/~oz/hash.html */
    uint hash = 5381;
    for(int i = 0; i < key_sz; i++){
	hash = ((hash << 5) + hash) + ((uint8_t*)key)[i];
    }
    return hash;
}

uint djb2_str(keyp key, size_t key_sz){
    /* djb2 hashing function from http://www.cse.yorku.ca/~oz/hash.html */
    uint hash = 5381;
    for(int i = 0; i < key_sz && ((char*)key)[i]; i++){
	hash = ((hash << 5) + hash) + ((char*)key)[i];
    }
    return hash;
}

void htab_init(hashp ht, ht_hash_func h, size_t key_sz, size_t val_sz, ulong buf_cap) {
    ht->key_sz = key_sz;
    ht->val_sz = val_sz;
    ht->buf_cap = buf_cap;
    ht->buf_load = 0;
    ht->h = h;
    ht->cmp = (ht_cmp_func)memcmp;
    ht->keycpy = (ht_key_cpy_func)memcpy;
    memset(ht->buf, 0, buf_cap * htab_entry_size(ht));
}

hashp htab_new(ht_hash_func h, size_t key_sz, size_t val_sz, ulong buf_cap){
    hashp new = malloc(sizeof(htab_t) + htab_entry_size2(key_sz, val_sz) * buf_cap);
    htab_init(new, h, key_sz, val_sz, buf_cap);
    return new;
}

void htab_free_entries(hashp ht, void (*free)(void *)){
    htab_entry_t *bucket;
    size_t entry_sz = htab_entry_size(ht);
    valp val;
    for(int i = 0; i < ht->buf_cap; i++){
	bucket = (void*)&ht->buf[entry_sz * i];
	val = htab_del(ht, bucket->contents);
	free(val);
    }
}

bool htab_put(hashp ht, keyp key, valp val) {
    uint hash = ht->h(key, ht->key_sz);
    size_t entry_sz = htab_entry_size(ht);

    htab_entry_t *bucket = (void*)&ht->buf[entry_sz * (hash % ht->buf_cap)];
    for (int probes = 0; probes < MAX_PROBES; probes++) {
        if (!bucket->flags || (bucket->flags & HT_ENTRY_FLAGS_DELETED)) {
            /* bucket is empty; add the entry */
            bucket->flags |= HT_ENTRY_FLAGS_OCCUPIED;
            ht->buf_load++; /* increase the load */
            bucket->hash = hash;
            ht->keycpy(bucket->contents, key, ht->key_sz);
            memcpy(bucket->contents + ht->key_sz, val, ht->val_sz);
            return true;
        }
        /* pseudo-random probing */
        bucket = (void*)&ht->buf[entry_sz * (rand_r(&hash) % ht->buf_cap)];
    }
    /* we failed to put the element in the hash table */
    return false;
}

valp htab_get(hashp ht, keyp key) {
    uint hash = ht->h(key, ht->key_sz);
    size_t entry_sz = htab_entry_size(ht);

    htab_entry_t *bucket = (void*)&ht->buf[entry_sz * (hash % ht->buf_cap)];

    while (bucket->flags) {
        /* while the bucket is not empty; */
        if (!(bucket->flags & HT_ENTRY_FLAGS_DELETED) && hash == bucket->hash &&
            (ht->cmp(bucket->contents, key, ht->key_sz) == 0)) {
            /* we found it! */
            return bucket->contents + ht->key_sz;
        }
        bucket = (void*)&ht->buf[entry_sz * (rand_r(&hash) % ht->buf_cap)];
    }

    /* we failed to put the element in the hash table */
    return NULL;
}

valp htab_del(hashp ht, keyp key) {
    valp val = htab_get(ht, key);
    if (val == NULL)
        /* key was not in the hash table */
        return NULL;

    htab_entry_t *bucket = val - offsetof(htab_entry_t, contents) - ht->key_sz;
    bucket->flags =
        (bucket->flags & ~HT_ENTRY_FLAGS_OCCUPIED) | HT_ENTRY_FLAGS_DELETED;
    ht->buf_load--;
    return val;
}

bool htab_rehash(hashp old, hashp new) {
    size_t entsz = htab_entry_size(old);
    for (int i = 0; i < old->buf_cap; i++) {
	htab_entry_t *bucket = (void*)&old->buf[entsz * i];
        if (bucket->flags & HT_ENTRY_FLAGS_OCCUPIED){
	    /* bucket is occupied */
	    htab_put(new, bucket->contents, bucket->contents + old->key_sz);
	}
    }
    return true;
}

bool htab_rehash_deep(hashp old, hashp new, valp (*copy)(valp)) {
    size_t entsz = htab_entry_size(old);
    for (int i = 0; i < old->buf_cap; i++) {
	htab_entry_t *bucket = (void*)&old->buf[entsz * i];
        if (bucket->flags & HT_ENTRY_FLAGS_OCCUPIED){
	    /* bucket is occupied */
	    htab_put(new, bucket->contents, copy(bucket->contents + old->key_sz));
	}
    }
    return true;
}
