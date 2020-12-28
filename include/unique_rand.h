#ifndef UNIQUE_RAND_H
#define UNIQUE_RAND_H

typedef struct unique_rand *URand;

void ur_create(URand *ur, int min, int max);

int ur_get(URand ur);

void ur_reset(URand ur);

void ur_destroy(URand *ur);

void ur_print(URand ur);

#endif
