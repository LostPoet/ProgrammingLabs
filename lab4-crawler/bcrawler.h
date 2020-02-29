#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "csapp.h"
#include "public.h"

#define BUF_SIZE 256

// non-negative
static int current_id = 1;

// use this to sync queue
sem_t mutex_send;
sem_t mutex_recv;

// a queue implementation
// this queue doesn't care about how you performing actions on P_TYPE
typedef struct P_TYPE {
    int id;
    char buf[BUF_SIZE];
} P_TYPE;

typedef struct QUEUE {
    struct QUEUE_NODE *front;
    struct QUEUE_NODE *rear;
} QUEUE;

typedef struct QUEUE_NODE {
    P_TYPE *pointer;
    struct QUEUE_NODE *next;
} QUEUE_NODE;

QUEUE* init_queue(sem_t *mutex) {
    QUEUE *result = (QUEUE *)malloc(sizeof(QUEUE));
    result->front = result->rear = (QUEUE_NODE *)malloc(sizeof(QUEUE_NODE));
    result->front->next = NULL;
    result->front->pointer = NULL;
    Sem_init(mutex, 0, 1);
    return result;
}

// the element must be stored in the first place
void enqueue(QUEUE *queue, P_TYPE *element, sem_t *mutex) {
    QUEUE_NODE *new_node;
    if (!(new_node = (QUEUE_NODE *)malloc(sizeof(QUEUE_NODE)))) {
        printf("Out of memory!\n");
        exit(0);
    }
    P(mutex);
    new_node->pointer = element;
    new_node->next = NULL;
    queue->rear->next = new_node;
    queue->rear = new_node;
    V(mutex);
}

P_TYPE* dequeue(QUEUE *queue, sem_t *mutex) {
    QUEUE_NODE *p;
    P_TYPE *result;
    if (queue->front == queue->rear)
        return NULL;
    P(mutex);
    p = queue->front->next;
    result = p->pointer;
    queue->front->next = p->next;
    if (p == queue->rear)
        queue->rear = queue->front;
    free(p);
    V(mutex);
    return result;
}

void clear_queue(QUEUE *queue) {
    QUEUE_NODE *p, *q;
    queue->rear = queue->front;
    p = queue->front->next;
    queue->front->next = NULL;

    while (p) {
        q = p;
        p = p->next;
        free(q);
    }
}

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
// return non-negative --> new url has been added, return id
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
        return -1;
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