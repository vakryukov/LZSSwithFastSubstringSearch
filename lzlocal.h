#ifndef _LZSS_LOCAL_H
#define _LZSS_LOCAL_H

#include <limits.h>

#define OFFSET_BITS     12
#define LENGTH_BITS     4

#if (((1 << (OFFSET_BITS + LENGTH_BITS)) - 1) > UINT_MAX)
#error "Size of encoded data greater than size of an unsigned int"
#endif

#define WINDOW_SIZE     (1 << OFFSET_BITS)

#define MAX_UNCODED     2
#define MAX_CODED       ((1 << LENGTH_BITS) + MAX_UNCODED)

#define ENCODED     0       
#define UNCODED     1       

typedef struct encoded_string_t
{
    unsigned int offset;    
    unsigned int length;    
} encoded_string_t;

#define Wrap(value, limit) \
    (((value) < (limit)) ? (value) : ((value) - (limit)))

int InitializeSearchStructures(void);
int ReplaceChar(const unsigned int charIndex, const unsigned char replacement);

encoded_string_t FindMatch(const unsigned int windowHead,
    const unsigned int uncodedHead);

#endif      /* ndef _LZSS_LOCAL_H */
