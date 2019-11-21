#include "../public.h"
#include <string.h>
#include <time.h>

typedef struct {
    char *p;
    int count;
} PATTERN;

int cmp(const void *pa, const void *pb) {
    return ((PATTERN*)pb)->count - ((PATTERN*)pa)->count;
}

int main() {
    FILE *pf = fopen("pattern_bf_kmp.txt", "r");
    FILE *sf = fopen("string.txt", "r");
    FILE *rf = fopen("strstr-result.txt", "w");

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

    char *string_storage = (char *)bupt_malloc(STRING_SIZE);
    char *sp = string_storage;
    while ((*sp++ = fgetc(sf)) != EOF)
        ;

    printf("Start matching:|");
    fflush(stdout);
    int timer = 0;
    // match the string
    clock_t start = clock();
    for (int i = 0; i < BF_KMP_PATTERN_LINES; i++) {
        timer++;
        patterns[i].count = 0;
        pp = patterns[i].p;
        sp = string_storage;
        while (*sp != '\0') {
            global_stats.cmpnum++;
            if (*pp == '\0')
                patterns[i].count++;
            if (*pp == *sp) {
                ++pp;
                ++sp;
            } else {
                sp = sp - (pp - patterns[i].p) + 1;
                pp = patterns[i].p;
            }
        }
        if (*pp == '\0')
            patterns[i].count++;
        printf("%s %d\n", patterns[i].p, patterns[i].count);
        fflush(stdout);
        if (timer % 150 == 0) {
            printf(">");
            fflush(stdout);
        }
    }
    clock_t end = clock();
    printf("|\n");

    // output with quick sort
    qsort(patterns, BF_KMP_PATTERN_LINES, sizeof(PATTERN), cmp);
    for (int i = 0; i < BF_KMP_PATTERN_LINES; ++i)
        fprintf(rf, "%s %d\n", patterns[i].p, patterns[i].count);

    fprintf(rf, "%dKiB %ldK", global_stats.mem / 1024, global_stats.cmpnum / 1000);
    printf("Total searching time(with I/O): %.0lfs\n", (double)((end-start) / CLOCKS_PER_SEC));
}