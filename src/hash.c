#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../include/hash.h"

struct Hashtable {
    unsigned long int bucketSize;
    unsigned long int capacity;

    pointer (*createValue )(pointer);

    int (*cmp )(pointer, pointer);

    void (*printValue)(pointer);

    unsigned long int (*hash)(pointer, unsigned long int);

    unsigned long int (*destroy)(pointer);

    pointer *table;
};


/***Private functions***/

void getCount(pointer bucket, unsigned long int bucketSize, unsigned long int *count) {
    memcpy(count, bucket + bucketSize - sizeof(unsigned long int) - sizeof(pointer), sizeof(unsigned long int));
}

void setCount(pointer bucket, unsigned long int bucketSize, const unsigned long int count) {
    memcpy(bucket + bucketSize - sizeof(unsigned long int) - sizeof(pointer), &count, sizeof(unsigned long int));
}

void getNext(pointer bucket, unsigned long int bucketSize, pointer *next) {
    memcpy(next, bucket + bucketSize - sizeof(pointer), sizeof(pointer));
}

void setNext(pointer bucket, unsigned long int bucketSize, pointer next) {
    memcpy(bucket + bucketSize - sizeof(pointer), &next, sizeof(pointer));
}

void getValue(pointer bucket, unsigned long int offset, pointer *value) {
    memcpy(value, bucket + offset * sizeof(pointer), sizeof(pointer));
}

void setValue(pointer bucket, unsigned long int offset, pointer value) {
    memcpy(bucket + offset * sizeof(pointer), &value, sizeof(pointer));
}

unsigned long int getEmptySlots(unsigned long int bucketSize, unsigned long int count) {
    return (bucketSize - sizeof(pointer) - sizeof(unsigned long int)) / sizeof(pointer) - count;
}

pointer allocBucket(unsigned long int size) { 
    pointer bucket = malloc((size_t) size);
    if (bucket != NULL) {
        setNext(bucket, size, NULL);
    }
    return bucket;
}


/***Public functions***/

bool ht_init(Hashtable *ht,
             unsigned long int capacity,
             unsigned long int bucketSize,
             pointer (*createValue)(pointer),
             int (*cmp)(pointer, pointer),
             void (*printValue)(pointer),
             unsigned long (*hash)(pointer, unsigned long int),
             unsigned long (*destroy)(pointer)
) {
    assert(bucketSize >= sizeof(pointer) * 2 + sizeof(unsigned long int));
    assert(capacity > 0);
    int i;
    *ht = (Hashtable) malloc(sizeof(struct Hashtable));
    if ((*ht) != NULL) {
        (*ht)->bucketSize = bucketSize;
        (*ht)->capacity = capacity;
        (*ht)->createValue = createValue;
        (*ht)->cmp = cmp;
        (*ht)->printValue = printValue;
        (*ht)->hash = hash;
        (*ht)->destroy = destroy;
        (*ht)->table = malloc(sizeof(pointer) * capacity);
        if ((*ht)->table != NULL) {
            for (i = 0; i < capacity; i++) {
                (*ht)->table[i] = NULL;
            }
            return true;
        }

    }
    return false;
}

int ht_insert(Hashtable ht, pointer key, pointer valueParams, pointer *value) {
    unsigned long int index = 0, count = 0, slots = 0, slot = 0;
    pointer bucket = NULL, b = NULL, next = NULL, slotValue = NULL;
    assert(ht != NULL);
    assert(key != NULL);
    assert(value != NULL);
    index = ht->hash(key, ht->capacity);
    //printf("[%.3lu] ", index);
    bucket = ht->table[index];

    /* Check if current bucket exists */
    if (bucket == NULL) {
        slotValue = ht->createValue(valueParams);
        if (slotValue != NULL) {
            bucket = allocBucket(ht->bucketSize);
            setValue(bucket, 0, slotValue);
            *value = slotValue;
            //printf("--> [%p] ", *value);
            setCount(bucket, ht->bucketSize, 1);
            ht->table[index] = bucket;
        } else {
            return false;
        }
    } else {
        //printf(" --> !!! Collision !!! ");
        next = bucket;

        /* Check each bucket to detect possibly duplicate values and
         * determine where is the target slot to write the new value.*/
        while (next != NULL) {
            getCount(bucket, ht->bucketSize, &count);
            slots = getEmptySlots(ht->bucketSize, count);

            /*Get value for each slot of bucket*/
            for (slot = 0; slot < count; slot++) {
                getValue(bucket, slot, &slotValue);
                if (!ht->cmp(slotValue, key)) {
                    //printf(":::DUPLICATE::: ");
                    //printf("\n");
                    *value = slotValue;
                    return false;
                }
            }

            /*Get next pointer to determine if this bucket has an overflow bucket*/
            getNext(bucket, ht->bucketSize, &next);

            if (next != NULL) {
                bucket = next;
            }
        };

        /*Check if there are exists empty slots at the last bucket*/
        if (slots) {
            //printf(":::BUCKET [%p] HAS %lu SLOTS::: ", bucket, slots - 1);
            slotValue = ht->createValue(valueParams);
            if (slotValue != NULL) {
                setValue(bucket, slot, slotValue);
                *value = slotValue;
                setCount(bucket, ht->bucketSize, count + 1);
            } else {
                return false;
            }
        } else {
            //printf(":::BUCKET [%p] IS FULL::: ", bucket);
            slotValue = ht->createValue(valueParams);
            if (slotValue != NULL) {
                b = allocBucket(ht->bucketSize);
                //printf(" --> ALLOCATE BUCKET [%p] ", b);
                setNext(bucket, ht->bucketSize, b);
                setValue(b, 0, slotValue);
                *value = slotValue;
                setCount(b, ht->bucketSize, 1);
            } else {
                return false;
            }
        }
    }
    //printf("\n");
    return true;
}

pointer ht_get(Hashtable ht, pointer key) {
    unsigned long int index = 0, count = 0, slot = 0;
    pointer bucket = NULL, v = NULL, next = NULL;
    assert(ht != NULL);
    assert(key != NULL);
    index = ht->hash(key, ht->capacity);
    bucket = ht->table[index];

    /* Check if current bucket exists */
    if (bucket != NULL) {
        next = bucket;

        /* Check each bucket to determine where is the target slot.*/
        while (next != NULL) {
            getCount(bucket, ht->bucketSize, &count);

            /*Get value for each slot of bucket*/
            for (slot = 0; slot < count; slot++) {
                getValue(bucket, slot, &v);
                if (!ht->cmp(v, key)) {
                    return v;
                }
            }

            /*Get next pointer to determine if this bucket has an overflow bucket*/
            getNext(bucket, ht->bucketSize, &next);
            if (next != NULL) {
                bucket = next;
            }
        };
    }
    return NULL;
}

int ht_remove(Hashtable ht, pointer key, pointer valueParams, bool forceDestroyItem) {
    unsigned long int index = 0, count = 0, slot = 0, targetSlot = 0;
    pointer bucket = NULL, targetBucket = NULL, next = NULL, slotValue = NULL;
    assert(ht != NULL);
    assert(key != NULL);

    index = ht->hash(key, ht->capacity);
    //printf("[%.3lu] ", index);
    bucket = ht->table[index];

    /* Check if current bucket exists */
    if (bucket == NULL) {
        return false; /* Nothing to remove*/
    } else {
        next = bucket;

        /* Check each bucket to detect target value, resume until last bucket.*/
        while (next != NULL) {
            getCount(bucket, ht->bucketSize, &count);

            /* Get value for each slot of bucket*/
            for (slot = 0; slot < count; slot++) {
                getValue(bucket, slot, &slotValue);
                /* Check for target slot*/
                if (!ht->cmp(slotValue, valueParams)) {
                    targetBucket = bucket;
                    targetSlot = slot;
                    if (forceDestroyItem) {
                        ht->destroy(slotValue);
                    }
                    setValue(bucket, slot, NULL);
                    slotValue = NULL;
                }
            }

            /* Get next pointer to determine if this bucket has an overflow bucket*/
            getNext(bucket, ht->bucketSize, &next);

            if (next != NULL) {
                bucket = next;
            }
        };

        /* Move last found slotValue in targetSlotValue*/
        if (targetBucket != NULL) {
            setValue(targetBucket, targetSlot, slotValue);
            setValue(bucket, slot, NULL);
            getCount(bucket, ht->bucketSize, &count);
            if (count > 0) {
                setCount(bucket, ht->bucketSize, --count);
            }
        }
    }
    //printf("\n");
    return true;
}

void ht_destroy(Hashtable *ht, bool forceDestroyItem) {
    assert((*ht) != NULL);
    pointer next = NULL, slotValue = NULL, bucket = NULL;
    unsigned long int count = 0, i, slot;
    //printf("\n");
    for (i = 0; i < (*ht)->capacity; i++) {
        bucket = next = (*ht)->table[i];
        if (bucket != NULL) {
            //printf("[%.3lu] -->", i);

            while (next != NULL) {
                getCount(bucket, (*ht)->bucketSize, &count);

                /*Get value for each slot of bucket to destroy it.*/
                if (forceDestroyItem) {
                    for (slot = 0; slot < count; slot++) {
                        getValue(bucket, slot, &slotValue);
                        (*ht)->destroy(slotValue);
                        //printf("[%p] ", slotValue);
                    }
                }

                /*Get next pointer to determine if this bucket has an overflow bucket.*/
                getNext(bucket, (*ht)->bucketSize, &next);

                free(bucket);

                if (next != NULL) {
                    bucket = next;
                }
            };
            //printf("\n");
        }
    }
    free((*ht)->table);
    free(*ht);
}

void ht_print(Hashtable ht) {
    int i = 0, slot = 0;
    unsigned long int count = 0;
    pointer bucket = NULL, v = NULL;
    for (i = 0; i < ht->capacity; i++) {
        int bucketNum = 0;
        bucket = ht->table[i];
        /* Check each bucket to determine where is the target slot.*/
        while (bucket != NULL) {
            getCount(bucket, ht->bucketSize, &count);
            /*Get value for each slot of bucket*/
            for (slot = 0; slot < count; slot++) {
                getValue(bucket, slot, &v);
                ht->printValue(v);
            }
            /*Get next pointer to determine if this bucket has an overflow bucket*/
            getNext(bucket, ht->bucketSize, &bucket);

            bucketNum++;
        }
    }
}
