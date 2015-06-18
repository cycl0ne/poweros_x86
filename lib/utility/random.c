#include "utility.h"
#include "utility_funcs.h"

#define	RAND_MAX	0x7fffffff

int util_Rand(pUtility UtilBase)
{
	return ((UtilBase->SeedNext = UtilBase->SeedNext * 1103515245 + 12345) % ((UINT32)RAND_MAX + 1));
}

void util_SRand(pUtility UtilBase, UINT32 seed)
{
	UtilBase->SeedNext = seed;
}

#define NSHUFF 50       /* to drop some "seed -> 1st value" linearity */

void util_SRandom(pUtility UtilBase,UINT32 seed)
{
	int i;
	UtilBase->RandSeed = seed;
	for (i = 0; i < NSHUFF; i++) (void)Random();
}

/*
 * Pseudo-random number generator for randomizing the profiling clock,
 * and whatever else we might use it for.  The result is uniform on
 * [0, 2^31 - 1].
 */
INT32 util_Random(pUtility UtilBase)
{
	INT32 x, hi, lo, t;

	/*
	 * Compute x[n + 1] = (7^5 * x[n]) mod (2^31 - 1).
	 * From "Random number generators: good ones are hard to find",
	 * Park and Miller, Communications of the ACM, vol. 31, no. 10,
	 * October 1988, p. 1195.
	 */
	/* Can't be initialized with 0, so use another value. */
	if ((x = UtilBase->RandSeed) == 0) x = 123459876;
	hi = x / 127773;
	lo = x % 127773;
	t = 16807 * lo - 2836 * hi;
	if (t < 0) t += 0x7fffffff;
	UtilBase->RandSeed = t;
	return (t);
}
