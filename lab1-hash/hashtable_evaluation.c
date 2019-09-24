#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "GeneralHashFunctions.h"

#define SLEN 104
#define HLEN 701819

struct {
    int wordcount;
    int yescount;
    int mem;
    long cmpnum;
} global_stats = { 0, 0, 0, 0 };

void* bupt_malloc(size_t size) {
    if (size <= 0)
        return NULL;
    global_stats.mem += size;
    return malloc(size);
}

void delnl(char *s) {
    while (*s++ != '\0')
        ;
    *(s-2) = '\0';
}

typedef unsigned int (*Hash)(const char* , unsigned int);

struct pattern{
    char str[SLEN];
};

int main() {
    FILE *pf = fopen("patterns-127w.txt", "r");
    //FILE *rf = fopen("hasheval-result.txt", "w");
    struct pattern *pp = (struct pattern *)bupt_malloc(1280000 * sizeof(struct pattern));
    struct pattern *pi = pp;

    Hash hf = SDBMHash;
    int hash[HLEN];
    memset(hash, 0, HLEN * 4);

    int length = 0;
    while (fgets(pi->str, SLEN, pf)) {
        delnl(pi->str);
        ++length;
        ++pi;
    }
    printf("%d\n", length);

    int i;
    for (i = 0, pi = pp; i < length; ++i, ++pi) {
        hash[hf(pi->str, strlen(pi->str)) % HLEN]++;
    }

    /*
    for (i = 0; i < HLEN; ++i) {
        printf("%d ", hash[i]);
        if (i % 100000 == 0)
            printf("\n");
    }
    printf("\n");
    */


    double rate, average;
    int r = 0, a = 0;
    int max = hash[0];
    for (i = 0; i < HLEN; ++i) {
        if (hash[i] > max)
            max = hash[i];
        if (hash[i] > 0)
            r++;
        a += hash[i];
    }
    rate = (double)r / (double)HLEN;
    average = (double)a / (double)HLEN;
    printf("| %d | %lf | %d | %f |\n", HLEN, rate, max, average);
    return 0;
}