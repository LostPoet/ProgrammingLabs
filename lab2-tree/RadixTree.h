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
    if (ulen == 0) {
        global_stats.cmpnum++;
        if (!p)
            return 0;
        else if ((long)p == 1)
            return 1;
        else
            return p->branch;
    }
    while (!((long)p & 1) && p) {
        global_stats.cmpnum++;
        int diretion = ((bitmask >> 2 * (p->pos%4)) & ubytes[p->pos/4]) >> 2 * (3 - p->pos%4);
        if (p->pos >= ulen * 4)
            return 0;
        if (p->pos == (4*ulen - 1)) {
            if (!((p->branch >> diretion) & 1))
                return 0;
            else {
                global_stats.cmpnum++;
                if ((long)p->child[diretion] & 1)
                    return 0 == string_cmp(u, (char *)p->child[diretion] - 1);
                else {
                    node *q = (node *)p->child[diretion];
                    while (!((long)q & 1)) {
                        for (int i = 0; i < 4; ++i)
                            if (q->child[i]) {
                                global_stats.cmpnum++;
                                q = q->child[i];
                                break;
                            }    
                    }
                    char *x = (char *)malloc(ulen + 1);
                    memcpy(x, (char *)q - 1, ulen + 1);
                    *(x+ulen) = '\0';
                    int result = (0 == string_cmp(u, x));
                    free(x);
                    return result;
                }
            }
        }
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
            // use t->root == 1 as a special mark
            *div = (char*)*div + 1;
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
            if (!*div || (long)*div & 1) {
                global_stats.node++;
                node *newnode = (node *)bupt_malloc(sizeof(node));
                newnode->pos = i * 4 + j;
                direction = ((bitmask >> 2 * j) & ubytes[i]) >> 2 * (3-j);
                *div = newnode;
                div = newnode->child + direction;
                // when a string ends at the edge
                if (i == (ulen-1) && j == 3) {
                    newnode->branch += 1 << direction;
                    char *x = (char *)malloc(ulen + 1);;
                    memcpy(x, ubytes, ulen + 1);
                    *div = (void *)((char *)x + 1);
                }
            } else {
                node *p = (node *)*div;
                direction = ((bitmask >> 2 * j) & ubytes[i]) >> 2 * (3-j);
                div = p->child + direction;
                // when a string ends inside tree
                if (i == (ulen-1) && j == 3) {
                    if (!*div) {
                        p->branch += 1 << direction;
                        char *x = (char *)malloc(ulen + 1);
                        memcpy(x, ubytes, ulen + 1);
                        *div = (void *)((char *)x + 1);
                    } else if (!((p->branch >> direction) & 1))
                        p->branch += 1 << direction;
                    else return 0;
                }
            }
        }
    }
    return 1;
}

node *orphan(node* n) {
    node *result = NULL;
    int child_num = 0;
    for (int i = 0; i < 4; ++i) {
        if (n->child[i]) {
            child_num++;
            result = n->child[i];
        }
    }
    if (child_num == 1)
        return result;
    else
        return NULL;
}

void shrink(node *n, void **parent) {
    node *child = NULL;
    if (!n || (long)n & 1)
        return;
    if ((child = orphan(n)) && n->branch == 0) {
        *parent = child;
        free(n);
        global_stats.node--; 
        global_stats.mem -= sizeof(node);
        shrink(child, parent);
    } else {
        for (int i = 0; i < 4; ++i)
            if (n->child[i] && ((long)n->child[i]%2==0))
                shrink(n->child[i], n->child + i);
    }
}
