#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>

#define BIT(a, n) (((a) >> (n)) & 1u)

#define NO_IMPL { fprintf(stderr, "NOT YET IMPLEMENTED\n"); exit(-5); }

#endif
