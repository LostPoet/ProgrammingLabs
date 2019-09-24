#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "public.h"

int main() {
    FILE *pf = fopen("patterns-127w.txt", "r");
    FILE *wf = fopen("words-98w.txt", "r");
    FILE *rf = fopen("array-result.txt", "w");
    struct pattern *pp = (struct pattern *)bupt_malloc(STORAGE * sizeof(struct pattern));
    struct pattern *pi = pp;
    struct pattern *pe;
    
    // load patterns and initialize pattern.str field
    int length = 0;
    while (bupt_getline(pi->str, pf)) {
        ++length;
        ++pi;
    }
    pe = pi - 1;

    printf("Length of patterns: %d\n", length);
    
    // search
    int i;
    struct word *pw = (struct word *)bupt_malloc(sizeof(struct word));
    time_t start = time(NULL);
    while (bupt_getline(pw->str, wf)) {
        global_stats.wordcount++;
        for (i = 0, pi = pe; i < length; ++i, --pi) {
            if (bupt_cmp(pw->str, pi->str) == 0) {
                fprintf(rf, "%s yes\n", pw->str);
                global_stats.yescount++;
                break;
            }
        }
        if (i == length)
            fprintf(rf, "%s no\n", pw->str);
    }

    fprintf(rf, "%dKiB %ld %d %d", global_stats.mem / 1024, global_stats.cmpnum, global_stats.wordcount, global_stats.yescount);
    printf("Total searching time(IO time included): %.1lfmins\n", (double)(difftime(time(NULL), start) / 60));
    return 0;
}

