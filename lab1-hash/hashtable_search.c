#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "public.h"
#include "GeneralHashFunctions.h"

int main() {
    FILE *pf = fopen("patterns-127w.txt", "r");
    FILE *wf = fopen("words-98w.txt", "r");
    FILE *rf = fopen("hash-result.txt", "w");
    struct hash_pattern *pp = (struct hash_pattern *)bupt_malloc(STORAGE* sizeof(struct hash_pattern));
    struct hash_pattern *pi = pp;

    // load hash_patterns and initialize hash_pattern.str field
    int length = 0;
    while (bupt_getline(pi->str, pf)) {   
        ++length;
        ++pi;
    }
    printf("Length of hash_patterns: %d\n", length);

    // assign hashfunc
    hash_function hf = APHash;
    struct hash_pattern **hash = (struct hash_pattern **)bupt_malloc(HLEN * sizeof(struct hash_pattern *));
    memset(hash, 0, HLEN * sizeof(struct hash_pattern *));

    // build hashtable
    int i;
    int pos;
    struct hash_pattern *end;
    for (i = 0, pi = pp; i < length; ++i, ++pi) {
        pos = hf(pi->str, strlen(pi->str)) % HLEN;
        if (!*(hash+pos)) {
            *(hash+pos) = pi;
            pi->next = NULL;
            continue;
        }
        for (end = *(hash+pos); end->next; end = end->next)
            ;
        end->next = pi;
        pi->next = NULL;
    }

    // load words and search
    struct word *pw = (struct word *)bupt_malloc(sizeof(struct word));
    clock_t timer = clock();
    while (bupt_getline(pw->str, wf)) {
        global_stats.wordcount++;
        pos = hf(pw->str, strlen(pw->str)) % HLEN;
        if (!(pi = *(hash+pos)))
            fprintf(rf, "%s no\n", pw->str);
        for (; pi; pi = pi->next) {
            if (bupt_cmp(pw->str, pi->str) == 0) {
                fprintf(rf, "%s yes\n", pw->str);
                global_stats.yescount++;
                break;
            }
        }
        if (*(hash+pos) && !pi)
            fprintf(rf, "%s no\n", pw->str);
    }
    fprintf(rf, "%dKiB %ld %d %d", global_stats.mem / 1024, global_stats.cmpnum, global_stats.wordcount, global_stats.yescount);
    printf("Total searching time(IO time included): %.0lfms\n", (double)((clock()-timer) * 1000 / CLOCKS_PER_SEC));
    return 0;
}