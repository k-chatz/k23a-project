#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "../include/unique_rand.h"

struct unique_rand {
    int min;
    int max;
    int *values;
    int length;
};

/***Private functions***/

void static inline swap(int *r1, int *r2) {
    int t = *r1;
    *r1 = *r2;
    *r2 = t;
}

/***Public functions***/

void ur_create(URand *ur, int min, int max) {
    assert(max > min);
    assert(*ur == NULL);
    *ur = (URand) malloc(sizeof(struct unique_rand));
    (*ur)->min = min;
    (*ur)->max = max;
    (*ur)->length = max - min + 1;
    (*ur)->values = malloc((*ur)->length * sizeof(int));
    for (int i = 0; i < (*ur)->length; i++) {
        (*ur)->values[i] = i + min;
    }
}

void ur_reset(URand ur) {
    ur->length = (ur->max - ur->min + 1);
    for (int i = 0; i < ur->length; i++) {
        ur->values[i] = ur->min + i;
    }
}

int ur_get(URand ur) {
    unsigned int seed = 123456;
    int i = 0;
    if (ur->length > 0) {
        i = rand_r(&seed) % ur->length;
        --ur->length;
        swap(&ur->values[i], &ur->values[ur->length]);
        return (int) ur->values[ur->length];
    } else {
        return ur->min - 1;
    }
}

void ur_destroy(URand *ur) {
    assert(ur != NULL);
    free((*ur)->values);
    free(*ur);
    *ur = NULL;
}

void ur_print(URand ur) {
    unsigned int x = (ur->max - ur->min + 1);
    for (int i = 0; i < x; i++) {
        printf("values[%d]=%d\n", i, ur->values[i]);
    }
}
