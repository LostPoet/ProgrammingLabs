#include "ac_auto.h"
#include "hash.h"
#include <string.h>
#include <math.h>
#include <time.h>

int hash_size() {
    int size = 0;
    ENTRY **hash = hashtable;
    for (int i = 0; i < HASH_LENGTH; ++i)
        for (ENTRY *e = *(hash + i); e; e = e->next)
            size++;
    return size;
}

int main() {
    FILE *pf = fopen("pattern.txt", "r");
    FILE *sf = fopen("string.txt", "r");
    FILE *rf = fopen("ac_auto-result.txt", "w");

    NODE state_machine;
    memset(&state_machine, 0, sizeof(NODE));
    state_machine.fs = &state_machine;
    state_machine.matchlist = (PATTERN *)1;

    // construct the state machine
    int length;
    char *pattern_storage = (char *)bupt_malloc(PATTERN_SIZE);
    memset(pattern_storage, 0, PATTERN_SIZE);
    char *pp = pattern_storage;
    int count = 0;
    while (length = bupt_getline(pp, pf)) {
        if (build_pattern(pp, &state_machine)) {
            pp += length;
            count++;
        }
    }
    printf("Distinct patterns: %d\n",count);
    set_fs(&state_machine);

    // match the string
    clock_t start = clock();
    count = 0;
    int c;
    NODE *next_state = &state_machine;
    PATTERN *result;
    while ((c = fgetc(sf)) != '\0') {
        result = processing(&next_state, c & 0xFF);
        while (result) {
            if (put(result->s))
                count++;
            result = result->next;
        }
    }
    clock_t end = clock();
    printf("Distinct patterns in result: %d\n", count);
    printf("hash size: %d\n", hash_size());

    printf("Task finishes\n");
    // output
    int stride = 1;
    int group = HASH_LENGTH;
    while (group != 1) {
        group = ceil(group / 2.0);
        for (int i = 0; i < group; ++i) {
            if (i * 2 * stride + stride < HASH_LENGTH)
                merge(hashtable + i * 2 * stride, hashtable + i * 2 * stride + stride);
        }
        stride *= 2;
    }

    printf("Merge ends\n");
    for (ENTRY *entry = hashtable[0]; entry; entry = entry->next)
        fprintf(rf, "%s %d\n", entry->key, entry->value);

    fprintf(rf, "%dKiB %ldK", global_stats.mem / 1024, global_stats.cmpnum / 1024);
    printf("Total searching time: %.0lfs\n", (double)((end-start) / CLOCKS_PER_SEC));
}