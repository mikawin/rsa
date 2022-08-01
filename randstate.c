#include "randstate.h"

#include <stdio.h>
#include <stdint.h>
#include <gmp.h>

gmp_randstate_t state;

// This function initializes a global random state variable using a given seed.
void randstate_init(uint64_t seed) {
    gmp_randinit_mt(state);
    gmp_randseed_ui(state, seed);
}

// This function clears and frees the memory of the global random state variable.
void randstate_clear(void) {
    gmp_randclear(state);
}
