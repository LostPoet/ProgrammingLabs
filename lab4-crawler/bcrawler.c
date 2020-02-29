#include "bcrawler.h"
#include "csapp.h"
#include <pcre.h>
#include <event2/event.h>
#include <event2/util.h>

// pcre definition
#define OVECCOUNT 30 /* should be a multiple of 3 */

// non-negative
// int current_id = 0;

// readonly data
char *host = "10.108.106.165", *port = "80";
char *url_prefix = "/news.sohu.com";
FILE *output;

// Two synchronized queue
QUEUE *send_queue, *recv_queue;

// structure to pass args from send_thread to write event
typedef struct ARGS {
    struct event_base *base;
    P_TYPE *url;
} ARGS;

// callback funcs
void socket_read_cb(int fd, short events, void *arg) {
    ARGS *a = ((ARGS *)arg);
    rio_t rio;
    char buf[MAXLINE];
    rio_readinitb(&rio, fd);
    memset(buf, 0, MAXLINE);

    // pcre definition
    const char *error;
    int erroffset;
    int ovector[OVECCOUNT];
    char *pattern = "<a.+?href=\"http://news.sohu.com(.+?)\".*>";

    pcre *re = pcre_compile(pattern, 0, &error, &erroffset, NULL);
    if (re == NULL)
        printf("PCRE compilation failed at offset %d: %s\n", erroffset, error);

    // a temp buf to store sub match
    //printf("asd\n");
    int ri;
    while ((ri = Rio_readlineb(&rio, buf, MAXLINE)) > 0) {
        if (pcre_exec(re, NULL, buf, strlen(buf), 0, 0, ovector, OVECCOUNT) < 2)
            continue;
        P_TYPE *new_url = (P_TYPE *)malloc(sizeof(P_TYPE));
        memset(new_url, 0, sizeof(P_TYPE));
        // this id indicates current url is pointed by whom
        new_url->id = a->url->id;
        memcpy(new_url->buf, buf + ovector[2], ovector[3] - ovector[2]);
        enqueue(recv_queue, new_url, &mutex_recv);
        printf("C\n");
    }
    if (ri == 0)
        printf("EOF when Rio_readlineb\n");
    else 
        printf("ERROR when Rio_readlineb\n");
    Close(fd);
    free(a->url);
    free(a);
}

void socket_write_cb(int fd, short events, void *arg) {
    ARGS *a = arg;
    char content[MAXLINE];
    memset(content, 0, MAXLINE);

    sprintf(content, "GET %s%s HTTP/1.1\r\nHost: %s\r\nConnection: keep-alive\r\n\r\n", url_prefix, a->url->buf, host);
    Rio_writen(fd, content, strlen(content));

    // add read event
    struct event* ev_r = event_new(a->base, fd, EV_READ, socket_read_cb, a);
    event_add(ev_r, NULL);
}

void *send_thread(void *vargp) {
    struct event_base *base = (struct event_base *)vargp;
    P_TYPE *url = NULL;
    pthread_detach(pthread_self());
    while (1) {
        if (url = dequeue(send_queue, &mutex_send)) {
            int clientfd;
            clientfd = Open_clientfd(host, port);
            evutil_make_socket_nonblocking(clientfd);
            printf("ID %d:connect to server successfully\n", url->id);

            ARGS *a = (ARGS *)malloc(sizeof(ARGS));
            a->base = base;
            a->url = url;
            // add write event
            struct event *ev_w = event_new(base, clientfd, EV_WRITE, socket_write_cb, a);
            event_add(ev_w, NULL);
        }
    }
}

void *send_thread_once(void *vargp) {
    struct event_base *base = (struct event_base *)vargp;
    P_TYPE *url = NULL;
    if (url = dequeue(send_queue, &mutex_send)) {
        int clientfd;
        clientfd = Open_clientfd(host, port);
        //evutil_make_socket_nonblocking(clientfd);
        printf("ID %d:connect to server successfully\n", url->id);

        ARGS *a = (ARGS *)malloc(sizeof(ARGS));
        a->base = base;
        a->url = url;
        // add write event
        struct event *ev_w = event_new(base, clientfd, EV_WRITE, socket_write_cb, a);
        event_add(ev_w, NULL);
    }
}

void *recv_thread_once(void *vargp) {
    NODE *root = (NODE *)vargp;
    P_TYPE *url = NULL;
    if (url = dequeue(recv_queue, &mutex_recv)) {
        int ri;
        if ((ri = url_persistence(url->buf, root, url->id)) > 0) {
            printf("B\n");
            fprintf(output, "%s %d\n", url->buf, ri);
            P_TYPE *new_url = (P_TYPE *)malloc(sizeof(P_TYPE));
            memset(new_url, 0, sizeof(P_TYPE));
            new_url->id = ri;
            memcpy(new_url->buf, url->buf, strlen(url->buf));
            enqueue(send_queue, new_url, &mutex_send);
        }
    }
    free(url);
}

void *recv_thread(void *vargp) {
    NODE *root = (NODE *)vargp;
    P_TYPE *url = NULL;
    pthread_detach(pthread_self());
    while (1) {
        if (url = dequeue(recv_queue, &mutex_recv)) {
            int ri;
            printf("A\n");
            if ((ri = url_persistence(url->buf, root, url->id)) > 0) {
                fprintf(output, "%s %d\n", url->buf, ri);
                P_TYPE *new_url = (P_TYPE *)malloc(sizeof(P_TYPE));
                memset(new_url, 0, sizeof(P_TYPE));
                new_url->id = ri;
                memcpy(new_url->buf, url->buf, strlen(url->buf));
                enqueue(send_queue, new_url, &mutex_send);
            }
        }
        free(url);
    }
}

int main(int argc, char **argv) {
    // check args
    if (argc != 3) {
        printf("Usage: %s <first_url> <output_file>", argv[0]);
        exit(0);
    }

    // initialization
    char *first_url = argv[1];
    output = fopen(argv[2], "w");
    send_queue = init_queue(&mutex_send);
    recv_queue = init_queue(&mutex_recv);

    // write output in advance and add first url to send_queue
    fprintf(output, "%s %d\n", first_url, 0);
    P_TYPE *new_url = (P_TYPE *)malloc(sizeof(P_TYPE));
    new_url->id = 0;
    memcpy(new_url->buf, first_url, strlen(first_url) + 1);
    enqueue(send_queue, new_url, &mutex_send);

    // a trie for persistence
    NODE root;
    memset(&root, 0, sizeof(NODE));
    root.matchlist = (PATTERN *)1;

    // setup event control
    struct event_base* base = event_base_new();

    // creates two threads
    pthread_t tid[4];
    Pthread_create(tid, NULL, send_thread_once, base);
    Pthread_join(tid[0], NULL);
    //Pthread_create(tid + 1, NULL, recv_thread_once, &root);
    Pthread_create(tid + 3, NULL, recv_thread, &root);
    event_base_loop(base, EVLOOP_ONCE);
    //Pthread_join(tid[1], NULL);
    printf("loop once\n");
    Pthread_create(tid + 2, NULL, send_thread, base);
    //Pthread_create(tid + 3, NULL, recv_thread, &root);
    printf("main loop \n");
    event_base_dispatch(base);
  
    printf("finished \n");
    return 0;
}