#include <stdio.h>
#include <stdlib.h>

#define STORAGE 1270784
#define SLEN 100
#define WLEN 128
#define HLEN 701819

struct {
    int wordcount;
    int yescount;
    int mem;
    int node;
    long cmpnum;
} global_stats = { 0, 0, 0, 0, 0 };

void* bupt_malloc(size_t size) {
    if (size <= 0)
        return NULL;
    global_stats.mem += size;
    return malloc(size);
}

int bupt_memalign(void **memptr, size_t alignment, size_t size) {
    global_stats.mem += size;
    return posix_memalign(memptr, alignment, size);
}

int bupt_cmp(const void *src1, const void *src2) {
    const unsigned char *s1 = (const unsigned char *)src1;
    const unsigned char *s2 = (const unsigned char *)src2;

    while (('\0' != *s1) || ('\0' != *s2)) {
        global_stats.cmpnum++;
        if (*s1 < *s2)
            return -1;
        else if (*s1 > *s2)
            return 1;
        s1++;
        s2++;
    }

    // both utf8 strings matched
    return 0;
}

int string_cmp(const void *src1, const void *src2) {
    const unsigned char *s1 = (const unsigned char *)src1;
    const unsigned char *s2 = (const unsigned char *)src2;
    while (('\0' != *s1) || ('\0' != *s2)) {
        if (*s1 < *s2)
            return -1;
        else if (*s1 > *s2)
            return 1;
        s1++;
        s2++;
    }
    return 0;
}

int bupt_getline(char *s, FILE *stream) {
    char *p = s;
    while((*p=getc(stream))!=EOF && *p++!='\n')
        ;
    if (p != s)  
        *(p-1) = '\0';
    return p - s;
}

struct pattern {
    char str[SLEN];
};

struct word {
    char str[WLEN];
};