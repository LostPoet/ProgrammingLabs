#include<stdio.h>
#include "utf8.h"

int main() {
    char *a = "中国";
    char *b = "中国";
    char *c = "中国2";

    printf("%d\n", utf8cmp(a, b));
    printf("%d\n", utf8cmp(a, c));
    return 0;
}