#include <stdio.h>
#include <stdlib.h>

#ifndef PUBLIC_PATTERN_MATCH
#define PUBLIC_PATTERN_MATCH

struct {
    int mem;
} global_stats = {0};

void* bupt_malloc(size_t size) {
    if (size <= 0)
        return NULL;
    global_stats.mem += size;
    return malloc(size);
}

#endif