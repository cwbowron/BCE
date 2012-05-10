#include <stdlib.h>
#include <time.h>

#include "bce.h"

void seed_rng(long s)
{
    srand(s);
}

void seed_randoms()
{
    if (seed_with_time)
	seed = time(NULL);
    seed_rng(seed);
}

/* return a 32bit random number */
long getrandomnumber() {
    return rand();
}
