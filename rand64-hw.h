#ifndef RAND64_HW_H
#define RAND64_HW_H

#include <stdint.h>
#include <stdbool.h>
#include <limits.h>

bool rdrand_supported(void);
void hardware_rand64_init(void);
unsigned long long hardware_rand64(void);
void hardware_rand64_fini(void);

#endif // RAND64_HW_H