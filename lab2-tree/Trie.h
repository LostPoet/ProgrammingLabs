#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "public.h"

#define uint8 unsigned char
#define uint32 unsigned int

typedef struct {
    void *child[4];
    uint32 pos;
    uint8 branch;
} node;

typedef struct {
    void *root;
} tree;

// u isn't in t --> return 0
// else --> return none-zero
int search_tree(tree *t, const char *u) {
    const uint8 *ubytes = (uint8 *)u;
    const size_t ulen = strlen(u);
    uint8 bitmask = 0xC0;

    node *p = (node *)t->root;
    if (p && ulen == 0) {
        global_stats.cmpnum++;
        if (!p->branch)
            return 0;
        else
            return 1;
    }
    while (p) {
        global_stats.cmpnum++;
        if (p->pos >= ulen * 4)
            return 0;
        else if (p->pos == (4*ulen - 1)) {
            int diretion = ((bitmask >> 2 * (p->pos%4)) & ubytes[p->pos/4]) >> 2 * (3 - p->pos%4);
            if ((p->branch >> diretion) & 1)
                return 1;
            else
                return 0;
        }
        int diretion = ((bitmask >> 2 * (p->pos%4)) & ubytes[p->pos/4]) >> 2 * (3 - p->pos%4);
        p = (node *)p->child[diretion];
    }
    return 0;
}

// already have u --> 0
// success --> 1
int insert(tree *t, const char *u) {
    const uint8 *ubytes = (uint8 *)u;
    const size_t ulen = strlen(u);
    void **div = &t->root;
    uint8 bitmask = 0xC0;

    if (ulen == 0) {
        if (!*div) {
            global_stats.node++;
            node *newnode = (node *)bupt_malloc(sizeof(node));
            memset(newnode, 0, sizeof(node));
            newnode->pos = 1;
            newnode->branch = 1;
            *div = (void *)newnode;
            return 1;
        } else {
            node *p = (node *)*div;
            if (!p->branch) {
                p->branch++;
                return 1;
            }
            return 0;
        }
    }
    int direction;
    for (int i = 0; i < ulen; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (!*div) {
                global_stats.node++;
                node *newnode = (node *)bupt_malloc(sizeof(node));
                memset(newnode, 0, sizeof(node));
                newnode->pos = i * 4 + j;
                direction = ((bitmask >> 2 * j) & ubytes[i]) >> 2 * (3-j);
                if (i == (ulen-1) && j == 3)
                    newnode->branch += 1 << direction;
                *div = (void *)newnode;
                div = newnode->child + direction;
            } else {
                node *p = (node *)*div;
                direction = ((bitmask >> 2 * j) & ubytes[i]) >> 2 * (3-j);
                // when a string ends inside tree
                if (i == (ulen-1) && j == 3) {
                    if (!((p->branch >> direction) & 1)) {
                        p->branch += 1 << direction;
                        return 1;
                    } else
                        return 0;
                }
                div = p->child + direction;
            }
        }
    }
    return 1;
}