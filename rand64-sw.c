#include <limits.h>
#include <stdio.h>
#include <time.h>
#include "rand64-sw.h"

/* Input stream containing random bytes.  */
static FILE *urandstream;
static char *path = "booty";

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
