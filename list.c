#include "lzlocal.h"

#define NULL_INDEX      (WINDOW_SIZE + 1)

extern unsigned char slidingWindow[];
extern unsigned char uncodedLookahead[];

unsigned int lists[UCHAR_MAX];              
unsigned int next[WINDOW_SIZE];             

int InitializeSearchStructures(void)
{
    unsigned int i;

    for (i = 0; i < WINDOW_SIZE; i++)
    {
        next[i] = i + 1;
    }

    next[WINDOW_SIZE - 1] = NULL_INDEX;

    for (i = 0; i < 256; i++)
    {
        lists[i] = NULL_INDEX;
    }

    lists[slidingWindow[0]] = 0;
    return 0;
}

encoded_string_t FindMatch(const unsigned int windowHead,
    const unsigned int uncodedHead)
{
    encoded_string_t matchData;
    unsigned int i;
    unsigned int j;

    (void)windowHead;       
    matchData.length = 0;
    matchData.offset = 0;
    i = lists[uncodedLookahead[uncodedHead]];   

    while (i != NULL_INDEX)
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

        if (j >= MAX_CODED)
        {
            matchData.length = MAX_CODED;
            break;
        }

        i = next[i];    
    }

    return matchData;
}

static void AddChar(const unsigned int charIndex)
{
    unsigned int i;

    next[charIndex] = NULL_INDEX;

    if (lists[slidingWindow[charIndex]] == NULL_INDEX)
    {
        lists[slidingWindow[charIndex]] = charIndex;
        return;
    }

    i = lists[slidingWindow[charIndex]];
    while(next[i] != NULL_INDEX)
    {
        i = next[i];
    }

    next[i] = charIndex;
}

static void RemoveChar(const unsigned int charIndex)
{
    unsigned int i;
    unsigned int nextIndex;

    nextIndex = next[charIndex];       
    next[charIndex] = NULL_INDEX;

    if (lists[slidingWindow[charIndex]] == charIndex)
    {
        lists[slidingWindow[charIndex]] = nextIndex;
        return;
    }

    i = lists[slidingWindow[charIndex]];

    while(next[i] != charIndex)
    {
        i = next[i];
    }

    next[i] = nextIndex;
}

int ReplaceChar(const unsigned int charIndex, const unsigned char replacement)
{
    RemoveChar(charIndex);
    slidingWindow[charIndex] = replacement;
    AddChar(charIndex);

    return 0;
}
