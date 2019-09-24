#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "public.h"
#include "MurmurHash.h"

int main() {
    FILE *pf = fopen("patterns-127w.txt", "r");
    FILE *wf = fopen("words-98w.txt", "r");
    FILE *rf = fopen("bf-result.txt", "w");
    struct pattern *pp = (struct pattern *)bupt_malloc(STORAGE * sizeof(struct pattern));
    struct pattern *pi = pp;

    // load patterns and initialize pattern.str field
    int length = 0;
    while (bupt_getline(pi->str, pf)) {
        ++length;
        ++pi;
    }
    printf("Length of patterns: %d\n", length);

    // compute arguments
    double fp = 0.00001;
    int k, m, bytes;
    m = (int)(length * 1.44 * log2(1.0/fp));
    bytes = (int)ceil((double)m/8.0);
    k = (int)round(0.693 * m / length);
    printf("m: %d bytes: %d k: %d\n", m, bytes, k);

    // build filter
    int i, j, pos;
    char *filter = (char *)bupt_malloc(bytes);
    memset(filter, 0, bytes);
    for (i = 0, pi = pp; i < length; ++i, ++pi) {
        for (j = 0; j < k; ++j) {
            pos = MurmurHash2(pi->str, strlen(pi->str), j) % m;
            // set bit
            *(filter + pos/8) |= (char)(1 << (7 - (pos%8)));
        }
    }

    // load words and search
    struct word *pw = (struct word *)bupt_malloc(sizeof(struct word));
    clock_t timer = clock();
    while (bupt_getline(pw->str, wf)) {
        global_stats.wordcount++;
        for (j = 0; j < k; ++j) {
            global_stats.cmpnum++;
            pos = MurmurHash2(pw->str, strlen(pw->str), j) % m;
            // get bit
            if ((*(filter + pos/8) & (char)(1 << (7 - (pos%8)))) == 0) {
                fprintf(rf, "%s no\n", pw->str);
                break;
            }
        }
        if (j == k) {
            fprintf(rf, "%s yes\n", pw->str);
            global_stats.yescount++;
        }
    }

    fprintf(rf, "%dKiB %ld %d %d", global_stats.mem / 1024, global_stats.cmpnum, global_stats.wordcount, global_stats.yescount);
    printf("Total searching time(IO time included): %.0lfms\n", (double)((clock()-timer) * 1000 / CLOCKS_PER_SEC));
    return 0;
}