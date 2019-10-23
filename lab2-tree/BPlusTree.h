#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "public.h"

#define M 8

typedef struct B_TNode {
	int keynum;
	struct B_TNode *parent;
	char *key[M+1];
	struct B_TNode *ptr[M+1];
	int leaf;
	union {
		struct B_TNode* next;
	} lf;
} B_TNode;

typedef struct {
	B_TNode* data;
	B_TNode* root;
}  B_Tree;

typedef struct {
	B_TNode* pt;						
	int i;								
	int tag;
} Result;

int Search(B_TNode *p, const char *str) {
	int i;
	for (i=0; i<p->keynum && string_cmp(str, p->key[i])<=0; i++)
		;
	return i;
}

Result SearchB_Tree(B_Tree B_T, const char *str) {
	Result R = {NULL, 0, 0};
	B_TNode *p;
	int found, i;
	
	p = B_T.root;
	found = 0;
	i = 0;
	
	while (p && !found) {
		i = Search(p, str);
		if (p->leaf==1) {
			if (i>0 && string_cmp(str, p->key[i-1])==0) {
				found = 1;
				i--;
			}
			else
				break;
		}
		else {
			if(i>0)
				i--;
				
			p = p->ptr[i];		
		}
	}
	
	R.i = i;
	R.pt = p;
	
	if (found)
		R.tag = 1;
	else
		R.tag = 0;
		
	return R;
}

int bupt_search(B_TNode *p, const char *str) {
	int i;
	for (i=0; i<p->keynum && bupt_cmp(str, p->key[i])<=0; i++)
		;
	return i;
}

Result bupt_searchTree(B_Tree B_T, const char *str) {
	Result R = {NULL, 0, 0};
	B_TNode *p;
	int found, i;
	
	p = B_T.root;
	found = 0;
	i = 0;
	
	while (p && !found) {
		i = bupt_search(p, str);
		if (p->leaf==1) {
			if (i>0 && bupt_cmp(str, p->key[i-1])==0) {
				found = 1;
				i--;
			}
			else
				break;
		}
		else {
			if (i>0)
				i--;
			p = p->ptr[i];		
		}
	}
	
	R.i = i;
	R.pt = p;
	
	if (found)
		R.tag = 1;
	else
		R.tag = 0;
		
	return R;
}

void Insert(B_TNode *q, int i, char *str, B_TNode *ap) {
	int j;
	B_TNode *p;
	
	for (j=q->keynum; j>i; j--) {
		q->key[j] = q->key[j-1];
		q->ptr[j] = q->ptr[j-1];
	}
	
	q->key[i] = str;
	q->ptr[i] = ap;
	q->keynum++;
	
	p = q->parent;
	while (!i && p) {
		i = Search(p, q->key[1]);
		--i;
		p->key[i] = str;
		p = p->parent;
	}
}

void split(B_TNode *q, int s, B_TNode **ap) {	
	int i;
	
	global_stats.node++;
	*ap = (B_TNode *)bupt_malloc(sizeof(B_TNode));
	
	if (q->leaf==1) {
		(*ap)->leaf = 1;
		(*ap)->lf.next = q->lf.next;
		q->lf.next = *ap;
	}
	else
		(*ap)->leaf = 0;
	
	for (i=s+1; i<=M; i++) {
		(*ap)->key[i-s-1] = q->key[i];
		(*ap)->ptr[i-s-1] = q->ptr[i];
	}
	
	(*ap)->keynum = M - s;
	q->keynum = s + 1;
	
	(*ap)->parent = q->parent;
	
	for (i=0; i<(*ap)->keynum; i++) {
		if((*ap)->ptr[i])
			(*ap)->ptr[i]->parent = *ap;
	}
}

void NewRoot(B_Tree *B_T, B_TNode *q, char *x, B_TNode *ap) { 
	B_TNode *p;

	global_stats.node++;
	p = (B_TNode *)bupt_malloc(sizeof(B_TNode));
	p->keynum = 1;
	p->parent = NULL;
	p->key[0] = x;
	p->ptr[0] = ap;
	if (p->ptr[0])
		p->ptr[0]->parent = p;
		
	if (!q && B_T->root) {
		Insert(p, 0, B_T->root->key[0], B_T->root);	
		if (p->ptr[0])
			p->ptr[0]->parent = p;	
	}
	
	B_T->root = p;	
	
	if (B_T->data==NULL) {
		p->leaf = 1;
		p->lf.next = NULL;
		B_T->data = p;	
	}
	else
		p->leaf = 0;
}

int InsertB_Tree(B_Tree *B_T, char *str, B_TNode *q, int i) {
	char *x;
	B_TNode *ap;
	int finished;
	int s;
	
	x = str;
	ap = NULL;
	finished = 0;
	
	while (q && !finished) {
		Insert(q, i, x, ap);
		q->ptr[i] = ap;
		
		if (q->keynum<=M)			
			finished = 1;		
		else {
			s = (int)ceil((double)M/2);	
			split(q, s, &ap);
			x = q->key[s+1];			
			q = q->parent;
			
			if (q)
				i = Search(q, x); 
		}
	}
	
	if (!finished) 
		NewRoot(B_T, q, x, ap); 
	
	return 1;
}

int InsertKey(B_Tree *B_T, char *str) {
	Result R;
	
	R = SearchB_Tree(*B_T, str);
	
	if (R.tag==0) {
		InsertB_Tree(B_T, str, R.pt, R.i);
		return 1;	
	}
	else
		return 0;	
}
