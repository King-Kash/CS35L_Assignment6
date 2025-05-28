#define _XOPEN_SOURCE 600
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "options.h"
#include "rand64-sw.h"

int process_options(int argc, char **argv, long long *nbytes, long long *customsize, int *inputOptions, int *outputOptions)
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
    }
  
    printf("optind=%d and argc=%d \n", optind, argc);

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

  
    printf("inputOpt=%s, outputOpt=%s, optind=%d, nbytes=%lld\n",
           inputOpt, outputOpt, optind, *nbytes);

    //Set input options
    if (strcmp(inputOpt, "rdrand") == 0)
    {
        printf("inputOption was rdrand\n");
        *inputOptions = 1;
    }
    else if (strcmp(inputOpt, "mrand48_r") == 0)
    {
        printf("inputOption was mrand48_r\n");
        *inputOptions = 2;
    }
    else if (inputOpt[0] == '/') 
    {
        printf("inputOption was custom file\n");
        *inputOptions = 3;
    }
    else 
    {
        fprintf(stderr, "Invalid input option.");
        return 1;
    }


    //Set output options
    // if (strcmp(outputOpt, "stdio") == 0)
    // {
    //     printf("outputOption was stdio\n");
    //     *outputOptions = 1;
    // }
    // else if(false)
    // {
    //     printf("need to impliment still");
    // }


  


    return 0; // Exit with no error
}
