#include "bcrawler.h"
#include "csapp.h"
#include "persistence.h"
#include <pcre.h>
#include <event2/event.h>

// pcre definition
#define OVECCOUNT 30 /* should be a multiple of 3 */

int main(int argc, char **argv) {
    // check args
    if (argc != 3) {
        printf("Usage: %s <first_url> <output_file>", argv[0]);
        exit(0);
    }

    // socket definition
    int clientfd;
    char *host = "10.108.106.165", *port = "80";
    char *url_prefix = "/news.sohu.com";
    char *first_url = argv[1];
    FILE *output = fopen(argv[2], "w");
    char buf[MAXLINE], content[MAXLINE];
    rio_t rio;
    memset(content, 0, MAXLINE);

    // write output in advance
    fprintf(output, "%s %d\n", first_url, current_id++);

    // pcre definition
    pcre *re;
    const char *error;
    int erroffset;
    int ovector[OVECCOUNT];
    char *pattern = "<a.+?href=\"http://news.sohu.com(.+?)\".*>";

    // pcre compile
    re = pcre_compile(pattern, 0, &error, &erroffset, NULL);
    if (re == NULL) {
        printf("PCRE compilation failed at offset %d: %s\n", erroffset, error);
        return 1;
    }

    // a trie for persistence
    NODE root;
    memset(&root, 0, sizeof(NODE));
    root.matchlist = (PATTERN *)1;
    
    // a queue to handle url loop
    QUEUE *msgq = init_queue();
    P_TYPE *new_elem = (P_TYPE *)malloc(sizeof(P_TYPE));
    memcpy(new_elem->buf, first_url, strlen(first_url) + 1);
    enqueue(msgq, new_elem);

    // current id of url
    int id = 0;

    P_TYPE *cur; 
    while ((cur = dequeue(msgq))) {

        // setup socket connection
        clientfd = Open_clientfd(host, port);
        Rio_readinitb(&rio, clientfd);
        printf("socket connected!\n");

        // send HTTP request
        sprintf(content, "GET %s%s HTTP/1.1\r\nHost: %s\r\nConnection: keep-alive\r\n\r\n", url_prefix, cur->buf, host);
        Rio_writen(clientfd, content, strlen(content));
        printf("sending HTTP req to %s\n", cur->buf);
        free(cur);

        // make sure HTTP res code is 200
        char status[4];
        Rio_readlineb(&rio, buf, MAXLINE);
        for (int i = 9; i < 12; ++i)
            status[i - 9] = buf[i];
        status[4] = '\0';
        printf("%d\n", atoi(status));
        fflush(stdout);
        if (atoi(status) != 200) {
            ++id;
            Close(clientfd);
            continue;
        }

        // parsing HTTP response
        while (Rio_readlineb(&rio, buf, MAXLINE) > 0) {
            int ri;
            if (pcre_exec(re, NULL, buf, strlen(buf), 0, 0, ovector, OVECCOUNT) < 2)
                continue;
            buf[ovector[3]] = '\0';
            if ((ri = url_persistence(buf + ovector[2], &root, id)) > 0) {
                fprintf(output, "%s %d\n", buf + ovector[2], ri);
                fflush(output);
                new_elem = (P_TYPE *)malloc(sizeof(P_TYPE));
                memcpy(new_elem->buf,  buf + ovector[2], ovector[3] - ovector[2] + 1);
                enqueue(msgq, new_elem);
            }
        }
        ++id;
        Close(clientfd);
    }
    
    fprintf(output, "\n");
    dump_link(output, &root);
    free(re);
    exit(0);
}