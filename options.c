#define _XOPEN_SOURCE 600
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "options.h"
#include "rand64-sw.h"

int process_options(int argc, char **argv, long long *nbytes, long *customsize, int *inputOptions, int *outputOptions)
{
    //exit with error code 1 anytime an error is experienced
    char *inputOpt = "rdrand";
    char *outputOpt = "stdio";
    bool valid = false;

    int opt;
    //get opt returns -1 after no more arugments identified
    while((opt = getopt(argc, argv, "i:o:")) != -1){
        switch (opt) {
            case 'i':
                inputOpt = optarg;
                break;
            case 'o':
                outputOpt = optarg;
                break;
            default:
                fprintf(stderr, "usage: %s -i [INPUT_OPTION] -o [OUTPUT_OPTION] NBYTES\n", argv[0]);
                return 1;
        }
    }

    if (optind >= argc)
    {
        fprintf(stderr, "missing NBYTES, usage: %s -i [INPUT_OPTION] -o [OUTPUT_OPTION] NBYTES\n", argv[0]);
        return 1;
    }
  
    //check for the correct number of arguments. optind should be argc-1 at this point and should be pointing to NUMBYTES.
    if (optind < argc)
    {
        char *endptr;
        errno = 0;
        *nbytes = strtoll(argv[optind], &endptr, 10);
        if (errno)
            perror(argv[optind]);
        else
            valid = *endptr=='\0' && 0 <= *nbytes; //check nbytes is a positive number and that input string terminates correctly after digits
    }
    else{
        return 1;
    }

    if (!valid)
    {
        fprintf(stderr, "check the following: NBYTES is a positive integer, max of one arugment passed to -i or -o\nusage: %s -i [INPUT_OPTION] -o [OUTPUT_OPTION] NBYTES\n", argv[0]);
        return 1;
    }

    //Set input options
    if (strcmp(inputOpt, "rdrand") == 0)
    {
        *inputOptions = 1;
    }
    else if (strcmp(inputOpt, "mrand48_r") == 0)
    {
        *inputOptions = 2;
    }
    else if (inputOpt[0] == '/') 
    {
        software_rand64_fetch(inputOpt);
        *inputOptions = 3;
    }
    else 
    {
        fprintf(stderr, "Invalid input option. Default to hardware.");
        *inputOptions = 1;
        return 1;
    }


    //Set output options
    if (strcmp(outputOpt, "stdio") == 0)
    {
        *outputOptions = 1;
        valid = true;
    }
    else
    {
        char *endptr;
        errno = 0;
        // *customsize = strtoll(outputOpt, &endptr, 10);
        // if (errno)
        //     perror(outputOpt);
        // else
        //     valid = *endptr == '\0' && 0 <= *customsize;
        long long tmp = strtoll(outputOpt, &endptr, 10);
        valid = (errno == 0 && *endptr == '\0' && tmp > 0);
        if (valid)
            *customsize = tmp;

        if (!valid)
        {
            printf("check the following: N is a positive integer.\n");
            fprintf(stderr,
                     "Usage: %s [-i INPUT] [-o OUTPUT] NBYTES\n OUTPUT must be “stdio” or a positive integer.\n",
                     argv[0]);
            return 1;
        }
        *outputOptions = 2;


        // char *endptr;
        // long long val;
        // errno = 0;
        // val = strtoll(outputOpt, &endptr, 10);
        // if (errno || *endptr != '\0' || val <= 0)
        // {
        //     fprintf(stderr, "Invalid OUTPUT chunk size: %s\n", outputOpt);
        //     return 1;
        // }
        // *outputOptions = 2;
        // *customsize = val;
    }


  


    return 0; // Exit with no error
}
