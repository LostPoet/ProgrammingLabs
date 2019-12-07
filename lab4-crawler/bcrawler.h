#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 256

// a queue to help set the fail state
typedef struct P_TYPE {
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

QUEUE* init_queue() {
    QUEUE *result = (QUEUE *)malloc(sizeof(QUEUE));
    result->front = result->rear = (QUEUE_NODE *)malloc(sizeof(QUEUE_NODE));
    result->front->next = NULL;
    result->front->pointer = NULL;
    return result;
}

// the element must be stored in the first place
void enqueue(QUEUE *queue, P_TYPE *element) {
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

P_TYPE* dequeue(QUEUE *queue) {
    QUEUE_NODE *p;
    P_TYPE *result;
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

