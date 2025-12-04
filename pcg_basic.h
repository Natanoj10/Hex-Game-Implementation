/*
 * PCG Random Number Generation
 * Minimal C implementation
 */

#ifndef PCG_BASIC_H
#define PCG_BASIC_H

#include <stdint.h>

typedef struct {
   uint64_t state;
   uint64_t inc;
} pcg32_random_t;

void pcg32_srandom_r(pcg32_random_t* rng, uint64_t initstate, uint64_t initseq);
uint32_t pcg32_random_r(pcg32_random_t* rng);
uint32_t pcg32_boundedrand_r(pcg32_random_t* rng, uint32_t bound);

#endif
