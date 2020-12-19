#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "../include/unique_rand.h"

struct unique_rand {
    int min;
    int max;
    int *values;
    int index;
};

/***Private functions***/

void static inline swap(int *r1, int *r2) {
    int t = *r1;
    *r1 = *r2;
    *r2 = t;
}

/***Public functions***/

void ur_create(UniqueRand *ur, int min, int max) {
    assert(max > min);
    srand(time(0));
    *ur = (UniqueRand) malloc(sizeof(struct unique_rand));
    (*ur)->min = min;
    (*ur)->max = max;
    (*ur)->index = max - min + 1;
    (*ur)->values = malloc((*ur)->index * sizeof(int));
    for (int i = 0; i < (*ur)->index; i++) {
        (*ur)->values[i] = i + min;
    }
}

void ur_reset(UniqueRand ur) {
    for (int i = 0; i < (ur->max - ur->min + 1); i++) {
        ur->values[i] = ur->min + i;
    }
}

int ur_get(UniqueRand ur) {
    int number = -1;
    int x = 0;
    if (ur->index > 0) {
        x = rand() % ur->index;
        number = (int) ur->values[x];
        swap(&ur->values[x], &ur->values[--ur->index]);
    } else {
        printf("Error:\nCan't generate more random numbers in function");
        ur_reset(ur);
    }
    return number;
}

void ur_destroy(UniqueRand *ur) {
    assert(ur != NULL);
    free((*ur)->values);
    free(*ur);
    *ur = NULL;
}

void ur_print(UniqueRand ur) {
    unsigned int x = (ur->max - ur->min + 1);
    for (int i = 0; i < x; i++) {
        printf("values[%d]=%d\n", i, ur->values[i]);
    }
}
