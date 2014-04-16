#include <stdlib.h>
#include <stdio.h>
#include "pool.h"

typedef struct block block_t;
typedef struct cleanup cleanup_t;

int ctr = 0;

#define MALLOC my_malloc
#define FREE my_free

void *my_malloc(size_t st) {
	ctr++;
	return malloc(st);
}

void my_free(void *ptr) {
	ctr--;
	free(ptr);
}

struct block {
	char *tag;
	void *ptr;
	size_t size;
	block_t *next;
};

struct cleanup {
	char *tag;
	void *data;
	void (*fn)(void *);
	cleanup_t *next;
};

struct pool {
	char *tag;
	pool_t *parent;
	pool_t *next;
	pool_t *children;
	block_t *blocks;
	cleanup_t *cleanups;
};

void *_pool_internal_alloc(pool_t *p, void *ptr, size_t size, char *tag) {
	block_t *blk, *b = MALLOC(sizeof(block_t));
	b->next = NULL;
	b->ptr = ptr ? ptr: MALLOC(size);
	b->tag = tag;
	
	for (blk = p->blocks; blk->next; blk = blk->next);
	blk->next = b;

	return b->ptr;
}

void *pool_remember(pool_t *p, void *ptr, char *tag) {
	ctr++;
	return _pool_internal_alloc(p, ptr, 0, tag);
}

void *pool_alloc(pool_t *p, size_t size, char *tag) {
	return _pool_internal_alloc(p, NULL, size, tag);
}


void register_cleanup(pool_t *p, void (*fn)(void *), void *data, char *tag) {
	cleanup_t *ct, *c = MALLOC(sizeof(cleanup_t));
	c->next = NULL;
	c->fn = fn;
	c->data = data;
	c->tag = tag;
	
	for (ct = p->cleanups; ct->next; ct = ct->next);
	ct->next = c;
}

pool_t *pool_create(pool_t *parent, char *tag) {
	pool_t *p;
		
	p = MALLOC(sizeof(pool_t));
	p->tag = tag;
	p->next = NULL;
	p->children = MALLOC(sizeof(pool_t));
	p->children->next = NULL;
	p->children->tag = "child-list";
	p->blocks = MALLOC(sizeof(block_t));
	p->blocks->next = NULL;
	p->blocks->tag = "block-list";
	p->cleanups = MALLOC(sizeof(cleanup_t));
	p->cleanups->next = NULL;
	p->cleanups->tag = "cleanup-list";
	p->parent = parent;
	if (parent) {
		pool_t *tmp;
		for (tmp = parent->children; tmp->next; tmp = tmp->next);
		tmp->next = p;
	}
	return p;
}

void pool_destroy_tree(pool_t *p) {
	pool_t *tmp;
	block_t *blk;
	cleanup_t *cln;

	tmp = p->children->next;
	while (tmp) {
		pool_t *tmp2 = tmp->next;
		pool_destroy_tree(tmp);
		tmp = tmp2;
	}
	FREE(p->children);

	cln = p->cleanups->next;
	while (cln) {
		cleanup_t *cln2 = cln->next;
		cln->fn(cln->data);
		FREE(cln);
		cln = cln2;
	}
	FREE(p->cleanups);

	blk = p->blocks->next;
	while (blk) {
		block_t *blk2 = blk->next;
		FREE(blk->ptr);
		FREE(blk);
		blk = blk2;
	}
	FREE(p->blocks);
	FREE(p);
}
	

void pool_destroy(pool_t *p) {
	if (p->parent) {
		/* unhook from parent's chain */
		pool_t *tmp = p->parent->children;
		while (tmp) {
			if (tmp->next == p) {
				tmp->next = p->next;
				break;
			}
			tmp = tmp->next;
		}
	}
	pool_destroy_tree(p);
}
		
void fn1(void *s){
	printf("ran: %s\n", (char *)s);
}

int pool_debug_ctr() {
	return ctr;
}
int main_test() {
	pool_t *a, *b, *c, *d, *p = pool_create(NULL, "p");
	a = pool_create(p, "a");
	d = pool_create(a, "d");
	b = pool_create(a, "b");
	c = pool_create(b, "c");
	pool_destroy(d);
	d = pool_create(c, "d");
	printf("ctr: %d\n", ctr);
	pool_alloc(a, 10, "10 bytes in a");
	pool_alloc(a, 20, "20 bytes in a");
	printf("ctr: %d\n", ctr);
	register_cleanup(c, fn1, (char *)"testing123", "clnup-1");
	register_cleanup(c, fn1, (char *)"soma", TAG);
	printf("ctr: %d\n", ctr);
	pool_destroy(b);
	printf("ctr: %d\n", ctr);
	pool_destroy(a);
	printf("ctr: %d\n", ctr);
	pool_destroy(p);
	printf("ctr: %d\n", ctr);
	return 0;
}