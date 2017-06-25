#ifndef OPTLIST_H
#define OPTLIST_H

#define    OL_NOINDEX    -1

typedef struct option_t
{
    char option;                
    char *argument;            
    int argIndex;               
    struct option_t *next;      
} option_t;

option_t *GetOptList(int argc, char *const argv[], char *const options);

void FreeOptList(option_t *list);

char *FindFileName(const char *const fullPath);

#endif  /* ndef OPTLIST_H */
