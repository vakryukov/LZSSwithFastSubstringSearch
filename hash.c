
#include "lzlocal.h"

typedef enum
{
    SRC_SLIDING_WINDOW, /* use slidingWindow */
    SRC_LOOKAHEAD       /* use uncodedLookahead */
} hash_src_t;

#define NULL_INDEX      (WINDOW_SIZE + 1)

#define HASH_SIZE       (WINDOW_SIZE >> 2)  /* size of hash table */

extern unsigned char slidingWindow[];
extern unsigned char uncodedLookahead[];

unsigned int hashTable[HASH_SIZE];          /* list head for each hash key */
unsigned int next[WINDOW_SIZE];             /* indices of next in hash list */

static unsigned int HashKey(const unsigned int offset,
    const hash_src_t hashSource)
{
    unsigned int i;
    unsigned int hashKey;

    hashKey = 0;

    if (SRC_LOOKAHEAD == hashSource)
    {
        /* string is in the lookahead buffer */
        for (i = 0; i < (MAX_UNCODED + 1); i++)
        {
            hashKey = (hashKey << 5) ^
                uncodedLookahead[Wrap((offset + i), MAX_CODED)];
            hashKey %= HASH_SIZE;
        }
    }
    else
    {
        /* string is in the sliding window */
        for (i = 0; i < (MAX_UNCODED + 1); i++)
        {
            hashKey = (hashKey << 5) ^
                slidingWindow[Wrap((offset + i), WINDOW_SIZE)];
            hashKey %= HASH_SIZE;
        }
    }

    return hashKey;
}

int InitializeSearchStructures()
{
    unsigned int i;

    for (i = 0; i < (WINDOW_SIZE - 1); i++)
    {
        next[i] = i + 1;
    }

    next[WINDOW_SIZE - 1] = NULL_INDEX;

    for (i = 0; i < HASH_SIZE; i++)
    {
        hashTable[i] = NULL_INDEX;
    }

    hashTable[HashKey(0, SRC_SLIDING_WINDOW)] = 0;

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

    i = hashTable[HashKey(uncodedHead, SRC_LOOKAHEAD)];
    j = 0;

    while (i != NULL_INDEX)
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

        i = next[i];    /* try next in list */
    }

    return matchData;
}

static void AddString(const unsigned int charIndex)
{
    unsigned int i;
    unsigned int hashKey;

    next[charIndex] = NULL_INDEX;

    hashKey = HashKey(charIndex, SRC_SLIDING_WINDOW);

    if (hashTable[hashKey] == NULL_INDEX)
    {
        /* this is the only character in it's list */
        hashTable[hashKey] = charIndex;
        return;
    }

    /* find the end of the list */
    i = hashTable[hashKey];

    while(next[i] != NULL_INDEX)
    {
        i = next[i];
    }

    /* add new character to the list end */
    next[i] = charIndex;
}

static void RemoveString(const unsigned int charIndex)
{
    unsigned int i;
    unsigned int hashKey;
    unsigned int nextIndex;

    nextIndex = next[charIndex];        /* remember where this points to */
    next[charIndex] = NULL_INDEX;

    hashKey = HashKey(charIndex, SRC_SLIDING_WINDOW);

    if (hashTable[hashKey] == charIndex)
    {
        /* we're deleting a list head */
        hashTable[hashKey] = nextIndex;
        return;
    }

    /* find character pointing to ours */
    i = hashTable[hashKey];

    while(next[i] != charIndex)
    {
        i = next[i];
    }

    /* point the previous next */
    next[i] = nextIndex;
}

int ReplaceChar(const unsigned int charIndex, const unsigned char replacement)
{
    unsigned int firstIndex;
    unsigned int i;

    if (charIndex < MAX_UNCODED)
    {
        firstIndex = (WINDOW_SIZE + charIndex) - MAX_UNCODED;
    }
    else
    {
        firstIndex = charIndex - MAX_UNCODED;
    }

    /* remove all hash entries containing character at char index */
    for (i = 0; i < (MAX_UNCODED + 1); i++)
    {
        RemoveString(Wrap((firstIndex + i), WINDOW_SIZE));
    }

    slidingWindow[charIndex] = replacement;

    /* add all hash entries containing character at char index */
    for (i = 0; i < (MAX_UNCODED + 1); i++)
    {
        AddString(Wrap((firstIndex + i), WINDOW_SIZE));
    }

    return 0;
}
