#ifndef RAND64_SW_H
#define RAND64_SW_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

void software_rand64_init(void);
unsigned long long software_rand64(void);
void software_rand64_fini(void);
void software_rand64_fetch(char *filepath);

void software_rand48_init(void);
unsigned long long software_rand48(void);
void software_rand48_fini(void);

#endif //RAND64_SW_H