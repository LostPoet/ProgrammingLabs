#include "public.h"
#include <string.h>
#include <time.h>

typedef struct {
    char *p;
    int count;
} PATTERN;

int cmp(const void *pa, const void *pb) {
    return ((PATTERN*)pb)->count - ((PATTERN*)pa)->count;
}

void get_next(char *t, int *next) {
    int i = 0,
        j = -1;
    next[0] = -1;
    while (i < (int)strlen(t)) {
        if (j == -1 || t[i] == t[j]) {
            i++;
            j++;
            next[i] = j;
        } else
            j = next[j];
    }
}

int main() {
    FILE *pf = fopen("pattern_bf_kmp.txt", "r");
    FILE *sf = fopen("string.txt", "r");
    FILE *rf = fopen("kmp-result.txt", "w");

    // read pattern and string
    int length;
    PATTERN patterns[BF_KMP_PATTERN_LINES] = {0};
    char *pattern_storage = (char *)bupt_malloc(BF_KMP_PATTERN_SIZE);
    memset(pattern_storage, 0, BF_KMP_PATTERN_SIZE);
    char *pp = pattern_storage; // reuesd later
    int index = 0;
    while (length = bupt_getline(pp, pf)) {
        patterns[index++].p = pp;
        pp += length;
    }

    fclose(pf);
    char *string_storage = (char *)bupt_malloc(STRING_SIZE);
    char *sp = string_storage;
    while ((*sp++ = fgetc(sf)) != EOF)
        ;

    printf("Start matching:|");
    fflush(stdout);
    // match the string
    int next[PATTERN_MAX];
    sp = string_storage; // hold there
    int l1 = strlen(sp); // also hold there
    int timer = 0;
    clock_t start = clock();
    for (int i = 0; i < BF_KMP_PATTERN_LINES; i++) {
        timer++;
        pp = patterns[i].p;
        get_next(patterns[i].p, next);
        int l2 = strlen(patterns[i].p);
        int k = 0,
            v = 0;
        while (k < l1) {
            global_stats.cmpnum++;
            if (v >= l2) {
                patterns[i].count++;
                v = 0;
            }
            if (v == -1 || sp[k] == pp[v]) {
                ++k;
                ++v;
            } else
                v = next[v];
        }
        if (timer % 150 == 0) {
            printf(">");
            fflush(stdout);
        }
    }
    clock_t end = clock();
    printf("|\n");

    // output with quike sort
    qsort(patterns, BF_KMP_PATTERN_LINES, sizeof(PATTERN), cmp);
    for (int i = 0; i < BF_KMP_PATTERN_LINES; ++i)
        fprintf(rf, "%s %d\n", patterns[i].p, patterns[i].count);

    fprintf(rf, "%dKiB %ldK", global_stats.mem / 1024, global_stats.cmpnum / 1024);
    printf("Total searching time(with I/O): %.0lfs\n", (double)((end-start) / CLOCKS_PER_SEC));
}