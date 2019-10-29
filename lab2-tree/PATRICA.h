#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "public.h"

#define uint8 unsigned char
#define uint32 unsigned int

typedef struct {
    void *child[2];
    uint32 byte;
    uint8 otherbits;
} node;

typedef struct {
    void *root;
} tree;

// u isn't in t --> return 0
// else --> return none-zero
int contains(tree *t, const char *u) {
    const uint8 *ubytes = (uint8 *)u;
    const size_t ulen = strlen(u);
    uint8 *p = (uint8 *)t->root;

    if (!p)
        return 0;
    while (1 & (long)p) {
        global_stats.cmpnum++;
        node *q = (node*)(p-1);
        uint8 c = 0;
        if (q->byte < ulen) 
            c = ubytes[q->byte];
        const int direction = (1+(q->otherbits|c)) >> 8;
        p = (uint8*) q->child[direction];
    }
    return 0 == string_cmp(u, (const char*)p);
}

// no more space --> 0
// already have u --> 1
// success --> 2
int insert(tree *t, const char *u) {
    const uint8 *ubytes = (uint8 *)u;
    const size_t ulen = strlen(u);
    uint8 *p = (uint8 *)t->root;

    if (!p) {
        char *x;
        int a = posix_memalign((void **)&x, sizeof(void *), ulen + 1);
        if (a)
            return 0;
        memcpy(x, u, ulen +1);
        t->root = x;
        return 2;
    }

    while (1 & (long)p) {
        node *q = (node*)(p-1);
        uint8 c = 0;
        if (q->byte < ulen) 
            c = ubytes[q->byte];
        const int direction = (1+(q->otherbits|c)) >> 8;
        p = (uint8*) q->child[direction];
    }

    uint32 newbyte;
    uint8 newotherbits;
    for (newbyte = 0; newbyte < ulen; ++newbyte) {
        if (p[newbyte] != ubytes[newbyte]) {
            newotherbits = p[newbyte] ^ ubytes[newbyte];
            goto different_byte_found;
        }
    }
    if (p[newbyte] != 0) {
        newotherbits = p[newbyte];
        goto different_byte_found;
    }
    return 1; different_byte_found:

    newotherbits |= newotherbits >> 1;
    newotherbits |= newotherbits >> 2;
    newotherbits |= newotherbits >> 4;
    newotherbits = (newotherbits & ~(newotherbits>>1)) ^ 255;
    uint8 c = p[newbyte];
    int newdirection = (1 + (newotherbits|c)) >> 8;

    node *newnode;
    if (bupt_memalign((void **) &newnode, sizeof(void *), sizeof(node)))
        return 0;
    char *x;
    if (posix_memalign((void **) &x, sizeof(void *), ulen + 1)) {
        free(newnode);
        return 0;
    }
    memcpy(x, ubytes, ulen + 1);
    newnode->byte = newbyte;
    newnode->otherbits = newotherbits;
    newnode->child[1 - newdirection] = x;

    void **wherep = &t->root;
    for(;;) {
        uint8 *p = (uint8 *)*wherep;
        if (!(1 & (long)p))
            break;
        node *q = (node *)(p - 1);
        if (q->byte > newbyte)
            break;
        if ((q->byte == newbyte) && q->otherbits > newotherbits)
            break;
        uint8 c = 0;
        if (q->byte < ulen)
            c = ubytes[q->byte];
        const int direction = (1 + (q->otherbits|c)) >> 8;
        wherep = q->child + direction;
    }
    newnode->child[newdirection] = *wherep;
    *wherep = (void *)(1 + (char *) newnode);
    return 2;
}
