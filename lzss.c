#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "lzlocal.h"
#include "bitfile.h"

unsigned char slidingWindow[WINDOW_SIZE];
unsigned char uncodedLookahead[MAX_CODED];

int EncodeLZSS(FILE *fpIn, FILE *fpOut)
{
    bit_file_t *bfpOut;
    encoded_string_t matchData;
    int c;
    unsigned int i;
    unsigned int len;

    unsigned int windowHead, uncodedHead;

    if ((NULL == fpIn) || (NULL == fpOut))
    {
        errno = ENOENT;
        return -1;
    }

    bfpOut = MakeBitFile(fpOut, BF_WRITE);

    if (NULL == bfpOut)
    {
        perror("Making Output File a BitFile");
        return -1;
    }

    windowHead = 0;
    uncodedHead = 0;

    memset(slidingWindow, ' ', WINDOW_SIZE * sizeof(unsigned char));

    for (len = 0; len < MAX_CODED && (c = getc(fpIn)) != EOF; len++)
    {
        uncodedLookahead[len] = c;
    }

    if (0 == len)
    {
        return 0;   
    }

    i = InitializeSearchStructures();

    if (0 != i)
    {
        return i;
    }

    matchData = FindMatch(windowHead, uncodedHead);

    while (len > 0)
    {
        if (matchData.length > len)
        {
            matchData.length = len;
        }

        if (matchData.length <= MAX_UNCODED)
        {
            BitFilePutBit(UNCODED, bfpOut);
            BitFilePutChar(uncodedLookahead[uncodedHead], bfpOut);

            matchData.length = 1;   
        }
        else
        {
            unsigned int adjustedLen;

            adjustedLen = matchData.length - (MAX_UNCODED + 1);

            BitFilePutBit(ENCODED, bfpOut);
            BitFilePutBitsNum(bfpOut, &matchData.offset, OFFSET_BITS,
                sizeof(unsigned int));
            BitFilePutBitsNum(bfpOut, &adjustedLen, LENGTH_BITS,
                sizeof(unsigned int));
        }

        i = 0;
        while ((i < matchData.length) && ((c = getc(fpIn)) != EOF))
        {
            ReplaceChar(windowHead, uncodedLookahead[uncodedHead]);
            uncodedLookahead[uncodedHead] = c;
            windowHead = Wrap((windowHead + 1), WINDOW_SIZE);
            uncodedHead = Wrap((uncodedHead + 1), MAX_CODED);
            i++;
        }

        while (i < matchData.length)
        {
            ReplaceChar(windowHead, uncodedLookahead[uncodedHead]);
            windowHead = Wrap((windowHead + 1), WINDOW_SIZE);
            uncodedHead = Wrap((uncodedHead + 1), MAX_CODED);
            len--;
            i++;
        }

        matchData = FindMatch(windowHead, uncodedHead);
    }

    BitFileToFILE(bfpOut);

   return 0;
}

int DecodeLZSS(FILE *fpIn, FILE *fpOut)
{
    bit_file_t *bfpIn;
    int c;
    unsigned int i, nextChar;
    encoded_string_t code;

    if ((NULL == fpIn) || (NULL == fpOut))
    {
        errno = ENOENT;
        return -1;
    }

    bfpIn = MakeBitFile(fpIn, BF_READ);

    if (NULL == bfpIn)
    {
        perror("Making Input File a BitFile");
        return -1;
    }

    memset(slidingWindow, ' ', WINDOW_SIZE * sizeof(unsigned char));

    nextChar = 0;

    while (1)
    {
        if ((c = BitFileGetBit(bfpIn)) == EOF)
        {
            break;
        }

        if (c == UNCODED)
        {
            if ((c = BitFileGetChar(bfpIn)) == EOF)
            {
                break;
            }

            putc(c, fpOut);
            slidingWindow[nextChar] = c;
            nextChar = Wrap((nextChar + 1), WINDOW_SIZE);
        }
        else
        {
            code.offset = 0;
            code.length = 0;

            if ((BitFileGetBitsNum(bfpIn, &code.offset, OFFSET_BITS,
                sizeof(unsigned int))) == EOF)
            {
                break;
            }

            if ((BitFileGetBitsNum(bfpIn, &code.length, LENGTH_BITS,
                sizeof(unsigned int))) == EOF)
            {
                break;
            }

            code.length += MAX_UNCODED + 1;

            for (i = 0; i < code.length; i++)
            {
                c = slidingWindow[Wrap((code.offset + i), WINDOW_SIZE)];
                putc(c, fpOut);
                uncodedLookahead[i] = c;
            }

            for (i = 0; i < code.length; i++)
            {
                slidingWindow[Wrap((nextChar + i), WINDOW_SIZE)] =
                    uncodedLookahead[i];
            }

            nextChar = Wrap((nextChar + code.length), WINDOW_SIZE);
        }
    }

    BitFileToFILE(bfpIn);

    return 0;
}
