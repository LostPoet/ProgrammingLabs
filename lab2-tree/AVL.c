#include "BBST.h"
#include <time.h>

int main() {
    FILE *pf = fopen("patterns-127w.txt", "r");
    FILE *wf = fopen("words-98w.txt", "r");
    FILE *rf = fopen("AVL-result.txt", "w");
    struct pattern *pp = (struct pattern *)malloc(STORAGE * sizeof(struct pattern));
    struct pattern *pi = pp;
    BBSTree t = NULL;
    int taller = 0;

    // load patterns and build AVL tree
    while (bupt_getline(pi->str, pf)) {
        InsertAVL(&t, pi->str, &taller);
        ++pi;
    }

    // search words in BBST
    struct word *pw = (struct word *)malloc(sizeof(struct word));
    clock_t timer = clock();
    while (bupt_getline(pw->str, wf)) {
        global_stats.wordcount++;
        if (SearchAVL(t, pw->str)) {
            fprintf(rf, "%s yes\n", pw->str);
            global_stats.yescount++;
        }
        else {
            fprintf(rf, "%s no\n", pw->str);
        }
    }

    fprintf(rf, "%d %dKiB %ldK %d %d", global_stats.node, global_stats.mem / 1024, global_stats.cmpnum / 1024, global_stats.wordcount, global_stats.yescount);
    printf("Total searching time(IO time included): %.0lfms\n", (double)((clock()-timer) * 1000 / CLOCKS_PER_SEC));
}