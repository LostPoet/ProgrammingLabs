#include "ac_auto.h"
#include "hash.h"
#include <string.h>
#include <time.h>

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
    char *pp = pattern_storage;
    int count = 0;
    while (length = bupt_getline(pp, pf)) {
        
        if (build_pattern(pp, &state_machine)) {
            pp += length;
            count++;
        }
    }
    printf("%d\n",count);
    set_fs(&state_machine);

    // match the string
    clock_t start = clock();
    int c;
    NODE *next_state = &state_machine;
    PATTERN *result;
    while ((c = fgetc(sf)) != '\0') {
        result = processing(&next_state, c & 0xFF);
        while (result) {
            put(result->s);
            result = result->next;
        }
    }
    clock_t end = clock();

    printf("Task finishes\n");
    // output
    for (int i = 1; i < HASH_LENGTH; ++i)
        merge(hashtable, hashtable + i);
    printf("Merge ends\n");
    for (ENTRY *entry = hashtable[0]; entry; entry = entry->next)
        fprintf(rf, "%s %d\n", entry->key, entry->value);

    fprintf(rf, "%dKiB %ldK", global_stats.mem / 1024, global_stats.cmpnum / 1024);
    printf("Total searching time: %.0lfms\n", (double)((end-start) * 1000 / CLOCKS_PER_SEC));
}