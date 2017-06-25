#include <stdio.h>
#include <ctype.h>
#include "lzlocal.h"

#define ROOT_INDEX      (WINDOW_SIZE + 1)
#define NULL_INDEX      (ROOT_INDEX + 1)

typedef struct tree_node_t
{
    unsigned int leftChild;
    unsigned int rightChild;
    unsigned int parent;
} tree_node_t;

extern unsigned char slidingWindow[];
extern unsigned char uncodedLookahead[];

tree_node_t tree[WINDOW_SIZE];      
unsigned int treeRoot;             

static void ClearNode(const unsigned int index);

static void AddString(const unsigned int charIndex);
static void RemoveString(const unsigned int charIndex);

static void PrintLen(const unsigned int charIndex, const unsigned int len);
static void DumpTree(const unsigned int root);

int InitializeSearchStructures(void)
{
    unsigned int i;

    for (i = 0; i < WINDOW_SIZE; i++)
    {
        ClearNode(i);
    }

    treeRoot = (WINDOW_SIZE - MAX_CODED) - 1;
    tree[treeRoot].parent = ROOT_INDEX;

    if (0)
    {
        DumpTree(NULL_INDEX);
    }

    return 0;
}

encoded_string_t FindMatch(const unsigned int windowHead,
    const unsigned int uncodedHead)
{
    encoded_string_t matchData;
    unsigned int i;
    unsigned int j;
    int compare;

    (void)windowHead;      
    matchData.length = 0;
    matchData.offset = 0;

    i = treeRoot;   
    j = 0;

    while (i != NULL_INDEX)
    {
        compare = slidingWindow[i] - uncodedLookahead[uncodedHead];

        if (0 == compare)
        {
            j = 1;

            while((compare = slidingWindow[Wrap((i + j), WINDOW_SIZE)] -
                uncodedLookahead[Wrap((uncodedHead + j), MAX_CODED)]) == 0)
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

        if (compare > 0)
        {
            i = tree[i].leftChild;
        }
        else
        {
            i = tree[i].rightChild;
        }
    }

    return matchData;
}

static int CompareString(const unsigned int index1, const unsigned int index2)
{
    unsigned int offset;
    int result = 0;

    for (offset = 0; offset < MAX_CODED; offset++)
    {
        result = slidingWindow[Wrap((index1 + offset), WINDOW_SIZE)] -
            slidingWindow[Wrap((index2 + offset), WINDOW_SIZE)];

        if (result != 0)
        {
            break;      
        }
    }

    return result;
}

static void FixChildren(const unsigned int index)
{
    if (tree[index].leftChild != NULL_INDEX)
    {
        tree[tree[index].leftChild].parent = index;
    }
    
    if (tree[index].rightChild != NULL_INDEX)
    {
        tree[tree[index].rightChild].parent = index;
    }
}

static void AddString(const unsigned int charIndex)
{
    int compare;
    unsigned int here;

    compare = CompareString(charIndex, treeRoot);

    if (0 == compare)
    {
        tree[charIndex].leftChild = tree[treeRoot].leftChild;
        tree[charIndex].rightChild = tree[treeRoot].rightChild;
        tree[charIndex].parent = ROOT_INDEX;
        FixChildren(charIndex);

        ClearNode(treeRoot);

        treeRoot = charIndex;
        return;
    }

    here = treeRoot;

    while(1)
    {
        if (compare < 0)
        {
            if (tree[here].leftChild != NULL_INDEX)
            {
                here = tree[here].leftChild;
            }
            else
            {
                tree[here].leftChild = charIndex;
                tree[charIndex].leftChild = NULL_INDEX;
                tree[charIndex].rightChild = NULL_INDEX;
                tree[charIndex].parent = here;
                FixChildren(charIndex);
                return;
            }
        }
        else if (compare > 0)
        {
            if (tree[here].rightChild != NULL_INDEX)
            {
                here = tree[here].rightChild;
            }
            else
            {
                /* we've hit the bottom */
                tree[here].rightChild = charIndex;
                tree[charIndex].leftChild = NULL_INDEX;
                tree[charIndex].rightChild = NULL_INDEX;
                tree[charIndex].parent = here;
                FixChildren(charIndex);
                return;
            }
        }
        else
        {
            tree[charIndex].leftChild = tree[here].leftChild;
            tree[charIndex].rightChild = tree[here].rightChild;
            tree[charIndex].parent = tree[here].parent;
            FixChildren(charIndex);

            if (tree[tree[here].parent].leftChild == here)
            {
                tree[tree[here].parent].leftChild = charIndex;
            }
            else
            {
                tree[tree[here].parent].rightChild = charIndex;
            }

            ClearNode(here);
            return;
        }

        compare = CompareString(charIndex, here);
    }
}

static void RemoveString(const unsigned int charIndex)
{
    unsigned int here;

    if (NULL_INDEX == tree[charIndex].parent)
    {
        return;     /* string isn't in tree */
    }

    if (NULL_INDEX == tree[charIndex].rightChild)
    {

        here = tree[charIndex].leftChild;
    }
    else if (NULL_INDEX == tree[charIndex].leftChild)
    {

        here = tree[charIndex].rightChild;
    }
    else
    {
        here = tree[charIndex].leftChild;

        while (tree[here].rightChild != NULL_INDEX)
        {
            here = tree[here].rightChild;
        }

        if (here != tree[charIndex].leftChild)
        {

            tree[tree[here].parent].rightChild = tree[here].leftChild;
            tree[tree[here].leftChild].parent = tree[here].parent;
            tree[here].leftChild = tree[charIndex].leftChild;
            tree[tree[charIndex].leftChild].parent = here;
        }

        tree[here].rightChild = tree[charIndex].rightChild;
        tree[tree[charIndex].rightChild].parent = here;
    }

    if (tree[tree[charIndex].parent].leftChild == charIndex)
    {
        tree[tree[charIndex].parent].leftChild = here;
    }
    else
    {
        tree[tree[charIndex].parent].rightChild = here;
    }

    tree[here].parent = tree[charIndex].parent;

    if (treeRoot == charIndex)
    {
        treeRoot = here;
    }

    ClearNode(charIndex);
}

int ReplaceChar(const unsigned int charIndex, const unsigned char replacement)
{
    unsigned int firstIndex, i;

    if (charIndex < MAX_CODED)
    {
        firstIndex = (WINDOW_SIZE + charIndex) - MAX_CODED;
    }
    else
    {
        firstIndex = charIndex - MAX_CODED;
    }

    for (i = 0; i <= MAX_CODED; i++)
    {
        RemoveString(Wrap((firstIndex + i), WINDOW_SIZE));
    }

    slidingWindow[charIndex] = replacement;

    for (i = 0; i <= MAX_CODED; i++)
    {
        AddString(Wrap((firstIndex + i), WINDOW_SIZE));
    }

    return 0;
}

static void ClearNode(const unsigned int index)
{
    const tree_node_t nullNode = {NULL_INDEX, NULL_INDEX, NULL_INDEX};

    tree[index] = nullNode;
}

static void PrintLen(const unsigned int charIndex, const unsigned int len)
{
    unsigned int i;

    for (i = 0; i < len; i++)
    {
        if (isprint(slidingWindow[Wrap((i + charIndex), WINDOW_SIZE)]))
        {
            putchar(slidingWindow[Wrap((i + charIndex), WINDOW_SIZE)]);
        }
        else
        {
            printf("<%02X>", slidingWindow[Wrap((i + charIndex), WINDOW_SIZE)]);
        }
    }
}

static void DumpTree(const unsigned int root)
{
    if (NULL_INDEX == root)
    {
        /* empty tree */
        return;
    }

    if (tree[root].leftChild != NULL_INDEX)
    {
        DumpTree(tree[root].leftChild);
    }

    printf("%03d: ", root);
    PrintLen(root, MAX_CODED);
    printf("\n");

    if (tree[root].rightChild != NULL_INDEX)
    {
        DumpTree(tree[root].rightChild);
    }
}
