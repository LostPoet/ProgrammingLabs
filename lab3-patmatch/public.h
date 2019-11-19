#include <stdio.h>
#include <stdlib.h>

#ifndef PUBLIC_PATTERN_MATCH
#define PUBLIC_PATTERN_MATCH

#define PATTERN_MAX 65          // max length of line
#define PATTERN_LINES 1281237   // distinct lines
#define PATTERN_SIZE 18087744   // characters inside pattern.txt
#define BF_KMP_PATTERN_MAX 23
#define BF_KMP_PATTERN_SIZE 13265 // characters inside pattern_bf_kmp.txt
#define BF_KMP_PATTERN_LINES 1500
#define STRING_SIZE 919943484   // characters inside string.txt

struct {
    int mem;
    long cmpnum;
} global_stats = {0, 0};

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

// A fatal error will occur if FILE dosn't have a carriage return in the end
int bupt_getline(char *s, FILE *stream) {
    char *p = s;
    while((*p=getc(stream))!=EOF && *p++!='\n')
        ;
    if (p != s)
        *(p-1) = '\0';
    return p - s;
}

#endif