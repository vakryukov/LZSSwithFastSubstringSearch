#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "lzss.h"
#include "optlist.h"


typedef enum
{
    ENCODE,
    DECODE
} modes_t;

int main(int argc, char *argv[])
{
    option_t *optList;
    option_t *thisOpt;
    FILE *fpIn;             
    FILE *fpOut;            
    modes_t mode;

    fpIn = NULL;
    fpOut = NULL;
    mode = ENCODE;

    optList = GetOptList(argc, argv, "cdi:o:h?");
    thisOpt = optList;

    while (thisOpt != NULL)
    {
        switch(thisOpt->option)
        {
            case 'c':
                mode = ENCODE;
                break;

            case 'd':
                mode = DECODE;
                break;

            case 'i':
                if (fpIn != NULL)
                {
                    fprintf(stderr, "Multiple input files not allowed.\n");
                    fclose(fpIn);

                    if (fpOut != NULL)
                    {
                        fclose(fpOut);
                    }

                    FreeOptList(optList);
                    return -1;
                }

                fpIn = fopen(thisOpt->argument, "rb");
                if (fpIn == NULL)
                {
                    perror("Opening input file");

                    if (fpOut != NULL)
                    {
                        fclose(fpOut);
                    }

                    FreeOptList(optList);
                    return -1;
                }
                break;

            case 'o':
                if (fpOut != NULL)
                {
                    fprintf(stderr, "Multiple output files not allowed.\n");
                    fclose(fpOut);

                    if (fpIn != NULL)
                    {
                        fclose(fpIn);
                    }

                    FreeOptList(optList);
                    return -1;
                }

                fpOut = fopen(thisOpt->argument, "wb");
                if (fpOut == NULL)
                {
                    perror("Opening output file");

                    if (fpIn != NULL)
                    {
                        fclose(fpIn);
                    }

                    FreeOptList(optList);
                    return -1;
                }
                break;

            case 'h':
            case '?':
                printf("Usage: %s <options>\n\n", FindFileName(argv[0]));
                printf("options:\n");
                printf("  -c : Encode input file to output file.\n");
                printf("  -d : Decode input file to output file.\n");
                printf("  -i <filename> : Name of input file.\n");
                printf("  -o <filename> : Name of output file.\n");
                printf("  -h | ?  : Print out command line options.\n\n");
                printf("Default: %s -c -i stdin -o stdout\n",
                    FindFileName(argv[0]));

                FreeOptList(optList);
                return 0;
        }

        optList = thisOpt->next;
        free(thisOpt);
        thisOpt = optList;
    }

    if (fpIn == NULL)
    {
        fpIn = stdin;
    }

    if (fpOut == NULL)
    {
        fpOut = stdout;
    }

    unsigned int start_time, end_time, search_time;
    if (mode == ENCODE)
    {
	start_time = clock();
        EncodeLZSS(fpIn, fpOut);
	end_time = clock();
    }
    else
    {
	start_time = clock();
        DecodeLZSS(fpIn, fpOut);
	end_time = clock();
    }
    search_time = end_time - start_time;
    double search_time_in_sec = search_time/1000000.0;
    printf("Time: ");
    printf("%f", search_time_in_sec);
    printf("sec.\n");
    fclose(fpIn);
    fclose(fpOut);
    return 0;
}
