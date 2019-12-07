#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "public.h"

// non-negative
int current_id = 0;

typedef struct PATTERN {
    struct LINK *linklist;
    int url_id;
    char *s;
    struct PATTERN *next;
} PATTERN;

typedef struct LINK {
    int link;   // the link linking others
    struct LINK *next;
} LINK;

typedef struct NODE {
    struct EDGE *edgelist;
    // will be only one match in this case
    struct PATTERN *matchlist; // also tag the root node
} NODE;

typedef struct EDGE{
    char c;
    struct NODE *p;
    struct EDGE *next;
} EDGE;

// state transfer
// return non-null --> there is a valid direction
// return null --> no valid direction
// return current_node --> no valid direction && current_node is root
NODE* state_goto(NODE *current_node, char c) {
    if (!current_node) {
        printf("Invalid Input In state_goto()!\n");
        exit(0);
    }
    for (EDGE *e = current_node->edgelist; e; e = e->next)
        if (e->c == c)
            return e->p;
    if ((long)current_node->matchlist == 1)
        return current_node;
    return NULL;
}

// once a string to build the trie
// return non-negative --> new url has been added
// return -1 -->  empty url || there has already been a same one
int url_persistence(char *str, NODE *start, int linked_by) {
    int result = -1;
    char *sp = str;
    NODE *current_node = start;
    while (*sp != '\0') {
        NODE *temp = state_goto(current_node, *sp);
        if (!temp | temp == current_node) {
            NODE *new_node = (NODE *)bupt_malloc(sizeof(NODE));
            memset(new_node, 0, sizeof(NODE));

            EDGE *new_edge = (EDGE *)bupt_malloc(sizeof(EDGE));
            memset(new_edge, 0, sizeof(EDGE));
            new_edge->c = *sp;
            new_edge->p = new_node;
            new_edge->next = current_node->edgelist;
            current_node->edgelist = new_edge;
            
            current_node = new_node;
        } else
            current_node = temp;
        sp++;
    }
    if (!current_node->matchlist) {
        PATTERN *new_pattern = (PATTERN *)bupt_malloc(sizeof(PATTERN));
        memset(new_pattern, 0, sizeof(PATTERN));
        // initialize new_pattern
        new_pattern->url_id = current_id++;
        result = new_pattern->url_id;
        new_pattern->s = str;

        // insert linked_by
        LINK *new_link = (LINK *)bupt_malloc(sizeof(LINK));
        new_link->link = linked_by;
        new_link->next = new_pattern->linklist;
        new_pattern->linklist = new_link;

        // insert new_pattern
        new_pattern->next = current_node->matchlist;
        current_node->matchlist = new_pattern;
    } else if ((long)current_node->matchlist > 1) {
        LINK *link_pointer = current_node->matchlist->linklist;

        while(link_pointer) {
            if (link_pointer->link == linked_by)
                return -1;
            link_pointer = link_pointer->next;
        }
        // insert linked_by
        LINK *new_link = (LINK *)bupt_malloc(sizeof(LINK));
        new_link->link = linked_by;
        new_link->next = current_node->matchlist->linklist;
        current_node->matchlist->linklist = new_link;
    }
    return result;
}

void dump_link(FILE *file, NODE *start) {
    if (!start)
        return;
    if ((long)start->matchlist > 1) {
        int id = start->matchlist->url_id;
        LINK *l = start->matchlist->linklist;
        while (l) {
            fprintf(file, "%d %d\n", l->link, id);
            l = l->next;
        }
    }
    for (EDGE *e = start->edgelist; e; e = e->next)
        dump_link(file, e->p);
}