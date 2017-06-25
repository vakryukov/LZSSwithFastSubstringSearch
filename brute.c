#include "lzlocal.h"

extern unsigned char slidingWindow[];
extern unsigned char uncodedLookahead[];

int InitializeSearchStructures(void)
{
    return 0;
}

encoded_string_t FindMatch(const unsigned int windowHead,
    unsigned int uncodedHead)
{
    encoded_string_t matchData;
    unsigned int i;
    unsigned int j;

    matchData.length = 0;
    matchData.offset = 0;
    i = windowHead;  
    j = 0;

    while (1)
    {
        if (slidingWindow[i] == uncodedLookahead[uncodedHead])
        {
            j = 1;

            while(slidingWindow[Wrap((i + j), WINDOW_SIZE)] ==
                uncodedLookahead[Wrap((uncodedHead + j), MAX_CODED)])
            {
                if (j >= MAX_CODED)
                {
                    break;
                }
                j++;
            }

            if (j > matchData.length)
            {
                matchData.length = j;
                matchData.offset = i;
            }
        }

        if (j >= MAX_CODED)
        {
            matchData.length = MAX_CODED;
            break;
        }

        i = Wrap((i + 1), WINDOW_SIZE);
        if (i == windowHead)
        {
            break;
        }
    }

    return matchData;
}

int ReplaceChar(const unsigned int charIndex, const unsigned char replacement)
{
    slidingWindow[charIndex] = replacement;
    return 0;
}
