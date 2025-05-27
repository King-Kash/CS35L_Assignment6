#include "rand64-hw.h"
#include <cpuid.h>
#include <immintrin.h>

//cupid struct to hold cpuid results
struct cpuid {
    unsigned eax, ebx, ecx, edx;
};


//cupid function
//struct and function do not share same namespace, thus we can use the same name
static struct cpuid //return type
cpuid(unsigned int leaf, unsigned int subleaf) {
    struct cpuid result;
    asm ("cpuid"
         : "=a" (result.eax), "=b" (result.ebx), //reads from registers eax and ebx
           "=c" (result.ecx), "=d" (result.edx) //reads from registers edx and ecx
         : "a" (leaf), "c" (subleaf)); //writes to registers eax and ecx
    return result;
}

_Bool //return type
rdrand_supported(void)
{
    struct cpuid extended = cpuid(1, 0);
    return (extended.ecx & bit_RDRND) != 0; //checks if the RDRAND bit is set in the ecx register using a mask and cpuid function
}

void //return type
hardware_rand64_init(void)
{
    // No initialization needed for hardware rand64
}

unsigned long long
hardware_rand64(void)
{
    unsigned long long int x;

    /* Work around GCC bug 107565
       <https://gcc.gnu.org/bugzilla/show_bug.cgi?id=107565>.  */
    x = 0;

    while (!_rdrand64_step(&x)) // Loop until we successfully get a random number
    // _rdrand64_step is a built-in function that attempts to generate a random number
        continue;
    return x;
}

/* Finalize the hardware rand64 implementation.  */
void
hardware_rand64_fini(void)
{
}
