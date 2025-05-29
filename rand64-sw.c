#include <limits.h>
#include <stdio.h>
#include <time.h>
#include "rand64-sw.h"

/* Input stream containing random bytes.  */
static FILE *urandstream;
static char *path = "/dev/random";

/* Initialize the software rand64 implementation.  */
void
software_rand64_init(void)
{
    urandstream = fopen(path, "r");
    if (!urandstream)
        abort();
}

/* Return a random value, using software operations.  */
unsigned long long
software_rand64(void)
{
    unsigned long long int x;
    if (fread(&x, sizeof x, 1, urandstream) != 1)
        abort();
    return x;
}

/* Finalize the software rand64 implementation.  */
void
software_rand64_fini(void)
{
    fclose(urandstream);
}

void
software_rand64_fetch(char *filepath)
{
    path = filepath;
}

//48 bit stream or something or other

// This has to be declared at file scope. It can not be declared within a function
static struct drand48_data randBuffer;

void
software_rand48_init(void)
{
    srand48_r(time(NULL), &randBuffer); // seed the generator with some entropy source
}

unsigned long long
software_rand48(void)
{
    unsigned long long x;
    long int hi, lo;
    lrand48_r(&randBuffer, &hi); //produce 32 bits each
    lrand48_r(&randBuffer, &lo);
    x = ((unsigned long long)hi << 32) | (unsigned long long)lo; //join hi and lo by shifting hi casting both as unsigned and combining into one (long long) casting.
    return x;
}

void software_rand48_fini(void)
{
}
