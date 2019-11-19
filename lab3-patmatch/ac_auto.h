#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "public.h"

typedef struct PATTERN {
    char *s;
    struct PATTERN *next;
} PATTERN;

typedef struct NODE {
    struct NODE *fs; // fail state
    struct EDGE *edgelist;
    struct PATTERN *matchlist; // also tag the root node
} NODE;

typedef struct EDGE{
    char c;
    struct NODE *p;
    struct EDGE *next;
} EDGE;

// state transfer
NODE* ac_goto(NODE *current_node, char c) {
    if (!current_node) {
        printf("Invalid Input In ac_goto()!\n");
        exit(0);
    }
    for (EDGE *e = current_node->edgelist; e; e = e->next)
        if (e->c == c)
            return e->p;
    if ((long)current_node->matchlist == 1)
        return current_node;
    return NULL;
}

// judge if the string is in the matchlist of current_node 
int no_such_string(const NODE *current_node, const char *string) {
    PATTERN *p;
    for (p = current_node->matchlist; p; p = p->next)
        if (p->s == string)
            return 0;
    return 1;
}

// once a string to build the state machine
// return 0 --> no state has been changed
// return 1 --> state changes
int build_pattern(char *str, NODE *start) {
    int flag = 0;
    char *sp = str;
    NODE *current_node = start;
    while (*sp != '\0') {
        NODE *temp = ac_goto(current_node, *sp);
        if (!temp | temp == current_node) {
            NODE *new_node = (NODE *)bupt_malloc(sizeof(NODE));
            memset(new_node, 0, sizeof(NODE));

            EDGE *new_edge = (EDGE *)bupt_malloc(sizeof(EDGE));
            new_edge->c = *sp;
            new_edge->p = new_node;
            new_edge->next = current_node->edgelist;
            current_node->edgelist = new_edge;
            
            current_node = new_node;
        } else
            current_node = temp;
        sp++;
    }
    if (current_node != start && no_such_string(current_node, str)) {
        flag = 1;
        PATTERN *new_pattern = (PATTERN *)bupt_malloc(sizeof(PATTERN));
        new_pattern->s = str;
        new_pattern->next = current_node->matchlist;
        current_node->matchlist = new_pattern;
    }
    return flag;
}

// a queue to help set the fail state
typedef struct QUEUE {
    struct QUEUE_NODE *front;
    struct QUEUE_NODE *rear;
} QUEUE;

typedef struct QUEUE_NODE {
    NODE *pointer;
    struct QUEUE_NODE *next;
} QUEUE_NODE;

QUEUE* init_queue() {
    QUEUE *result = (QUEUE *)malloc(sizeof(QUEUE));
    result->front = result->rear = (QUEUE_NODE *)malloc(sizeof(QUEUE_NODE));
    result->front->next = NULL;
    result->front->pointer = NULL;
    return result;
}

void enqueue(QUEUE *queue, NODE *element) {
    QUEUE_NODE *new_node;
    if (!(new_node = (QUEUE_NODE *)malloc(sizeof(QUEUE_NODE)))) {
        printf("Out of memory!\n");
        exit(0);
    }
    new_node->pointer = element;
    new_node->next = NULL;
    queue->rear->next = new_node;
    queue->rear = new_node;
}

NODE* dequeue(QUEUE *queue) {
    QUEUE_NODE *p;
    NODE *result;
    if (queue->front == queue->rear)
        return NULL;
    p = queue->front->next;
    result = p->pointer;
    queue->front->next = p->next;
    if (p == queue->rear)
        queue->rear = queue->front;
    free(p);
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

// as its name
void merge_matchlist(NODE *target, NODE *source) {
    PATTERN *sm = source->matchlist;
    while (sm && (long)sm != 1) {
        if (no_such_string(target, sm->s)) {
            PATTERN *new_pattern = (PATTERN *)bupt_malloc(sizeof(PATTERN));
            new_pattern->s = sm->s;
            new_pattern->next = target->matchlist;
            target->matchlist = new_pattern;
        }
        sm = sm->next;
    }
}

// set the fail state for the entire state machine
void set_fs(NODE *start) {
    QUEUE *Q = init_queue();
    // initialize the nodes in the first level
    for (EDGE *e = start->edgelist; e; e = e->next) {
        e->p->fs = start;
        enqueue(Q, e->p);
    }

    // set other nodes
    NODE *current_node;
    while (current_node = dequeue(Q)) {
        for (EDGE *e = current_node->edgelist; e; e = e->next) {
            NODE *fail_state = current_node->fs;
            while (!(e->p->fs = ac_goto(fail_state, e->c)))
                fail_state = fail_state->fs;
            merge_matchlist(e->p, e->p->fs);
            enqueue(Q, e->p);
        }
    }
    clear_queue(Q);
}

PATTERN* processing(NODE **current_state, char direction) {
    NODE *destiny;
    NODE *state = *current_state;
    while (!(destiny = ac_goto(state, direction))) {
        state = state->fs;
        global_stats.cmpnum++;
    }
    *current_state = destiny;
    if (destiny->matchlist && (long)destiny->matchlist != 1)
        return destiny->matchlist;
    else
        return NULL;
}