#include <stdio.h>
#include <stdlib.h>
#include "public.h"

// 平衡因子
#define LH +1
#define EH  0
#define RH -1

typedef struct BBSTNode {
	char *data;
	int bf;			
	struct BBSTNode *lchild;
	struct BBSTNode *rchild;
} BBSTNode;
typedef BBSTNode *BBSTree;

void L_Rotate(BBSTree *p) {
	BBSTree rc;
	
	rc = (*p)->rchild;
	(*p)->rchild = rc->lchild;
	rc->lchild = *p;
	*p = rc;	
}

void R_Rotate(BBSTree *p) {
	BBSTree lc;
	
	lc = (*p)->lchild;
	(*p)->lchild = lc->rchild;
	lc->rchild = *p;
	*p = lc;	
}

void LeftBalance(BBSTree *BBST) {
	BBSTree lc, rd;
	
	lc = (*BBST)->lchild;
	
	switch(lc->bf)
	{
		case LH:
			(*BBST)->bf = lc->bf = EH;
			R_Rotate(BBST);
			break;
		case RH:
			rd = lc->rchild;			
			switch(rd->bf)
			{
				case LH:
					(*BBST)->bf = RH;
					lc->bf = EH;
					break;
				case EH:
					(*BBST)->bf = lc->bf = EH;
					break;
				case RH:
					(*BBST)->bf = EH;
					lc->bf = LH;
					break;
			}			
			rd->bf = EH;
			L_Rotate(&(*BBST)->lchild);
			R_Rotate(BBST);
			break;
	}
}

void RightBalance(BBSTree *BBST) {
	BBSTree rc, ld;
	
	rc = (*BBST)->rchild;
	
	switch(rc->bf)
	{
		case RH:
			(*BBST)->bf = rc->bf = EH;
			L_Rotate(BBST);			
			break;

		case LH:
			ld = rc->lchild;			
			switch(ld->bf)
			{
				case LH:
					(*BBST)->bf = EH;
					rc->bf = RH;
					break;
				case EH:
					(*BBST)->bf = rc->bf = EH;
					break;
				case RH:
					(*BBST)->bf = LH;
					rc->bf = EH;
					break;
			}			
			ld->bf = EH;
			R_Rotate(&(*BBST)->rchild);
			L_Rotate(BBST);
			break;
	}
}

int InsertAVL(BBSTree *BBST, char *string, int *taller) {
	if (!(*BBST)) {
        ++global_stats.node;
		*BBST = (BBSTree)bupt_malloc(sizeof(BBSTNode));
		(*BBST)->data = string;
		(*BBST)->lchild = (*BBST)->rchild = NULL;
		(*BBST)->bf = EH;
		*taller = 1;
	} else {
		if (string_cmp(string, (*BBST)->data) == 0) {
			*taller = 0;
			return 0;
		} else if (string_cmp(string, (*BBST)->data) > 0) {
			if (!InsertAVL(&(*BBST)->lchild, string, taller))
				return 0;

			if (*taller) {
				switch((*BBST)->bf)
				{
					case LH:
						LeftBalance(BBST);
						*taller = 0;
						break;
					case EH:
						(*BBST)->bf = LH;
						*taller = 1;
						break; 
					case RH:
						(*BBST)->bf = EH;
						*taller = 0;
						break;
				}
			} 
		} else {
			if (!InsertAVL(&(*BBST)->rchild, string, taller))
				return 0;		

			if (*taller) {
				switch((*BBST)->bf)
				{
					case LH: 
						(*BBST)->bf = EH;
						*taller = 0;
						break;
					case EH: 
						(*BBST)->bf = RH;
						*taller = 1;
						break; 
					case RH:
						RightBalance(BBST);
						*taller = 0;
						break;
				}
			}
		}
	}
	return 1;
}

BBSTree SearchAVL(const BBSTree BBST, const char *word) {
	global_stats.cmpnum++;
	if (!BBST || (string_cmp(word, BBST->data) == 0))
		return BBST;
	else if (string_cmp(word, BBST->data) > 0)
		return SearchAVL(BBST->lchild, word);
	else
		return SearchAVL(BBST->rchild, word);
}