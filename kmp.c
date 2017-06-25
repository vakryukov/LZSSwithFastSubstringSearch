#include "lzlocal.h"

extern unsigned char slidingWindow[];
extern unsigned char uncodedLookahead[];

int InitializeSearchStructures(void)
{
    return 0;
}

static void FillTable(unsigned char *uncoded, int* kmpTable)
{
    int i, j; 

    kmpTable[0] = -1;
    kmpTable[1] = 0;
    i = 2;
    j = 0;

    while (i < MAX_CODED)
    {
        if (uncoded[i - 1] == uncoded[j])
        {
            j++;
            kmpTable[i] = j;
            i++;
        }
        else if (j > 0)
        {
            j = kmpTable[j];
        }
        else
        {
            kmpTable[i] = 0;
            i++;
        }
    }
}

encoded_string_t FindMatch(const unsigned int windowHead,
    const unsigned int uncodedHead)
{
    encoded_string_t matchData;
    unsigned int m;
    unsigned int i;
    int kmpTable[MAX_CODED]; 
    unsigned char localUncoded[MAX_CODED];  

    for (i = 0; i < MAX_CODED; i++)
    {
        localUncoded[i] =
            uncodedLookahead[Wrap((uncodedHead + i), MAX_CODED)];
    }

    FillTable(localUncoded, kmpTable);

    matchData.length = 0;
    matchData.offset = 0;
    m = 0;
    i = 0;

    while (m < WINDOW_SIZE)
    {
        if (localUncoded[i] ==
            slidingWindow[Wrap((m + i + windowHead), WINDOW_SIZE)])
        {
            i++;

            if (MAX_CODED == i)
            {
                matchData.length = MAX_CODED;
                matchData.offset = Wrap((m + windowHead), WINDOW_SIZE);
                break;
            }
        }
        else
        {
            if (i > matchData.length)
            {
                matchData.length = i;
                matchData.offset = Wrap((m + windowHead), WINDOW_SIZE);
            }
            m = m + i - kmpTable[i];

            if (kmpTable[i] > 0)
            {
                i = kmpTable[i];
            }
            else
            {
                i = 0;
            }
        }
    }

    return matchData;
}

int ReplaceChar(const unsigned int charIndex, const unsigned char replacement)
{
    slidingWindow[charIndex] = replacement;
    return 0;
}
