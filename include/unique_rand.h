#ifndef UNIQUE_RAND_H
#define UNIQUE_RAND_H

typedef struct unique_rand *UniqueRand;

void ur_create(UniqueRand *ur, int min, int max);

int ur_get(UniqueRand ur);

void ur_reset(UniqueRand ur);

void ur_destroy(UniqueRand *ur);

void ur_print(UniqueRand ur);

#endif
