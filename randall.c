/* Generate N bytes of random output.  */

/* When generating output this program uses the x86-64 RDRAND
   instruction if available to generate random numbers, falling back
   on /dev/random and stdio otherwise.

   This program is not portable.  Compile it with gcc -mrdrnd for a
   x86-64 machine.

   Copyright 2015, 2017, 2020 Paul Eggert

   This program is free software: you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation, either version 3 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

// #include <cpuid.h>
#include <errno.h>
#include <immintrin.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "rand64-hw.h"
#include "rand64-sw.h"
#include "output.h"
#include "options.h"

/* Main program, which outputs N bytes of random data.  */
int
main (int argc, char **argv)
{
  /* Check arguments.  */
  long long nbytes;
  int inputOption = 0;
  int outputOption = 0;
  long customsize;

  process_options(argc, argv, &nbytes, &customsize, &inputOption, &outputOption);

  /* If there's no work to do, don't worry about which library to use.  */
  if (nbytes == 0)
    return 0;

  /* Now that we know we have work to do, arrange to use the
     appropriate library.  */
  void (*initialize) (void);
  unsigned long long (*rand64) (void);
  void (*finalize) (void);

  if (inputOption == 1)
  {
    if (rdrand_supported())
    {
      initialize = hardware_rand64_init;
      rand64 = hardware_rand64;
      finalize = hardware_rand64_fini;
    }
    else
    {
      return 1;
    }

  }
  else if (inputOption == 2)
  {
    initialize = software_rand48_init;
    rand64 = software_rand48;
    finalize = software_rand48_fini;
  }
  else if (inputOption == 3)
  {
    initialize = software_rand64_init;
    rand64 = software_rand64;
    finalize = software_rand64_fini;
  }
  else 
  {
    printf("no input");
  }

  
  initialize ();
  int wordsize = sizeof rand64 ();
  int output_errno = 0;

  if (outputOption == 1)
  {
    do
    {
      unsigned long long x = rand64();
      int outbytes = nbytes < wordsize ? nbytes : wordsize; //this is similar logic to chunk
      if (!writebytes(x, outbytes))
      {
        output_errno = errno;
        break;
      }
      nbytes -= outbytes;
    } while (0 < nbytes);
  }
  else if (outputOption == 2)
  {
    //printf("N: %ld\n", customsize);
    char *buf = malloc(customsize);
    if (!buf)
    {
      perror("malloc");
      return 1;
    }
    long long remaining = nbytes;
    while (remaining > 0)
    {
      long long chunk = (remaining < customsize ? remaining : customsize);
      //fill the buffer
      long long pos = 0;
      while (pos < chunk)
      {
        unsigned long long x = rand64();
        for (int i = 7; i >= 0 && pos < chunk; i--)
        {
          buf[pos++] = (x >> (i * CHAR_BIT)) & 0xFF;
        }
      }
      //write the buffer
      char* ptr = buf;
      long long towrite = chunk;
      while (towrite > 0)
      {
        ssize_t k = write(1, ptr, towrite);
        if (k < 0)
        {
          output_errno = errno;
          break;
        }
        ptr += k;
        towrite -= k;
        remaining -=k;
      }
      if (output_errno)
      {
        break;
      }
    }
    free(buf);
  }

  // if (fclose (stdout) != 0)
  //   output_errno = errno;

  if (output_errno)
    {
      errno = output_errno;
      perror ("output");
    }

  finalize ();
  return !!output_errno;
}

/*
2 layers

declare the buffer check that it was intilized with customsize
remaining = nbytes
1. top layer - handles nbytes
until all nbytes have been written out so remaining > 0
  chunk = min of customsize or remaining to see how many bytes are needed for this iteration
  pass how many bytes need to be printed into the inner loop or a custom function (my own func)
  the inner loop will have handled initilizing the buffer so now we can print it
  we need to use the write function for this (be careful as write does not always output the expected number of bytes)
    ptr = buff (point to start of the buffer)
    towrite = chunk (number of bytes to write)
    while (towrite > 0)
    {
      k = write(1, ptr, towrite) //k = how many bytes were actually printed. may not be as many as we would like on just the first pass
      if ( k < 0)
      {
        error
      }
      ptr += k;
      towrtie -= k
      remaining -= k //this ensure we accurately capture how many bytes we actually printed out
    }

2. inside layer - handles chunks

in the old version we handled printing 8 bytes at a time but now we need to handle printing N bytes at a time.
pointer to track position inside of buffer
We need a loop in here to run while pos < chunk size
  generate 64 bits (8 bytes)
  loop through 64 bits and append one byte at a time to chunk
    conditions on loop:
      1. i < 8 only 64 bits = 8 bytes generated so make sure we dont try to append more than 8 bytes at a time
      2. pos < chunk in case within this appeneding loop we hit the end of the chunks
    iteration on loop:
      1. i++
  chunck size - number of bytes appended
Now the chunk is full so we can print the whole chunk

the number of bytes we need told to

Once all writing is done free the memory allocated to the buffer
*/

/* _____________________________ Hardware implementation. _____________________________ */

// /* Description of the current CPU.  */
// struct cpuid { unsigned eax, ebx, ecx, edx; };

// /* Return information about the CPU.  See <http://wiki.osdev.org/CPUID>.  */
// static struct cpuid
// cpuid (unsigned int leaf, unsigned int subleaf)
// {
//   struct cpuid result;
//   asm ("cpuid"
//        : "=a" (result.eax), "=b" (result.ebx),
// 	 "=c" (result.ecx), "=d" (result.edx)
//        : "a" (leaf), "c" (subleaf));
//   return result;
// }

// /* Return true if the CPU supports the RDRAND instruction.  */
// static _Bool
// rdrand_supported (void)
// {
//   struct cpuid extended = cpuid (1, 0);
//   return (extended.ecx & bit_RDRND) != 0;
// }

// /* Initialize the hardware rand64 implementation.  */
// static void
// hardware_rand64_init (void)
// {
// }

// /* Return a random value, using hardware operations.  */
// static unsigned long long
// hardware_rand64 (void)
// {
//   unsigned long long int x;

//   /* Work around GCC bug 107565
//      <https://gcc.gnu.org/bugzilla/show_bug.cgi?id=107565>.  */
//   x = 0;

//   while (! _rdrand64_step (&x))
//     continue;
//   return x;
// }

// /* Finalize the hardware rand64 implementation.  */
// static void
// hardware_rand64_fini (void)
// {
// }

/* _____________________________ Software implementation. _____________________________ */

// /* Input stream containing random bytes.  */
// static FILE *urandstream;

// /* Initialize the software rand64 implementation.  */
// static void
// software_rand64_init (void)
// {
//   urandstream = fopen ("/dev/random", "r");
//   if (! urandstream)
//     abort ();
// }

// /* Return a random value, using software operations.  */
// static unsigned long long
// software_rand64 (void)
// {
//   unsigned long long int x;
//   if (fread (&x, sizeof x, 1, urandstream) != 1)
//     abort ();
//   return x;
// }

// /* Finalize the software rand64 implementation.  */
// static void
// software_rand64_fini (void)
// {
//   fclose (urandstream);
// }

/* _____________________________ Main _____________________________ */

// static bool
// writebytes (unsigned long long x, int nbytes)
// {
//   do
//     {
//       if (putchar (x) < 0)
// 	return false;
//       x >>= CHAR_BIT;
//       nbytes--;
//     }
//   while (0 < nbytes);

//   return true;
// }
