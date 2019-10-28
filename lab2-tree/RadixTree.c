#include "RadixTree.h"
#include <time.h>

int main() {
    FILE *pf = fopen("patterns-127w.txt", "r");
    FILE *wf = fopen("words-98w.txt", "r");
    FILE *rf = fopen("RadixTree-result.txt", "w");
    struct pattern *pp = (struct pattern *)malloc(sizeof(struct pattern));
    tree *t = (tree *)malloc(sizeof(tree));
    t->root = NULL;

    // load patterns and build the radix tree
    while (bupt_getline(pp->str, pf))
        insert(t, pp->str) == 2;

    // compress the 4-way radix tree 
    shrink(t->root, &t->root);

    // search words in the radix tree
    struct word *pw = (struct word *)malloc(sizeof(struct word));
    clock_t timer = clock();
    while (bupt_getline(pw->str, wf)) {
        global_stats.wordcount++;
        if (search_tree(t, pw->str)) {
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