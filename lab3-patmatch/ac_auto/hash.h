#include "../public.h"
#include <string.h>

#define HASH_LENGTH 393241

typedef struct ENTRY {
    char *key;          // string pointer, no space will be allocated
    int value;          // times in this case
    struct ENTRY *next;
} ENTRY;

unsigned int APHash(const char* str, unsigned int len)
{
   unsigned int hash = 0xAAAAAAAA;
   unsigned int i    = 0;

   for(i = 0; i < len; str++, i++)
   {
      hash ^= ((i & 1) == 0) ? (  (hash <<  7) ^ (*str) * (hash >> 3)) :
                               (~((hash << 11) + ((*str) ^ (hash >> 5))));
   }

   return hash;
}

ENTRY *hashtable[HASH_LENGTH] = {0};

// keep the order according to value
// return 1 --> new key
// return 0 --> already exist
int put(char *s) {
    int pos = APHash(s, strlen(s)) % HASH_LENGTH;
    ENTRY *ep = hashtable[pos];
    while (ep && (ep->key != s))
        ep = ep->next;
    if (!ep) {
        ENTRY *new_entry = (ENTRY *)malloc(sizeof(ENTRY));
        new_entry->key = s;
        new_entry->value = 1;
        ENTRY *tail = hashtable[pos];
        while (tail && tail->next)
            tail = tail->next;
        if (!tail) {
            new_entry->next = hashtable[pos];
            hashtable[pos] = new_entry;
        } else {
            new_entry->next = tail->next;
            tail->next = new_entry;
        }
        return 1;
    } else {
        ep->value++;
        ENTRY *pre = hashtable[pos];
        ENTRY *last;
        if (pre == ep)
            ;
        else if (pre->value < ep->value) {
            for (last = pre; last->next != ep; last = last->next)
                ;
            // no matter if last == pre
            last->next = ep->next;
            ep->next = pre;
            hashtable[pos] = ep;
        } else {
            while (pre->value > ep->value && pre->next->value > ep->value)
                pre = pre->next;
            for (last = pre; last->next != ep; last = last->next)
                ;
            // also no matter if last == pre
            last->next = ep->next;
            ep->next = pre->next;
            pre->next = ep;
        }
    }
    return 0;
}

void merge(ENTRY **ea, ENTRY **eb) {
    ENTRY *pa = *ea,
          *pb = *eb,
          *lc = (ENTRY *)malloc(sizeof(ENTRY)); // with a head node
    memset(lc, 0, sizeof(ENTRY));
    ENTRY *pc = lc;
    if (!pa) {
        *ea = *eb;
        *eb = NULL;
        return;
    }
    while (pa && pb) {
        if (pa->value > pb->value) {
            pc->next = pa;
            pc = pa;
            pa = pa->next;
        } else {
            pc->next = pb;
            pc = pb;
            pb = pb->next;
        }
    }
    pc->next = pa ? pa : pb;
    *ea = lc->next;
    free(lc);
}