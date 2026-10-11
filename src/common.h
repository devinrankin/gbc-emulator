#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>

#define BIT(a, n) (((a) >> (n)) & 1u)
#define BIT_SET // todo
#define RANGE(a, b,c ) ((a >= b) && (a <= c)) 

#define NO_IMPL { fprintf(stderr, "NOT YET IMPLEMENTED\n"); exit(-5); }

enum {
    SIZE_KIB = 1024,
    SIZE_MIB = 1024 * SIZE_KIB
};

#endif
