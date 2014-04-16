#ifndef __POOL_H__
#define __POOL_H__
#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>

typedef struct pool pool_t;

#define S1(a) #a
#define S2(a) S1(a)
#define TAG __FILE__ ":" __FUNCTION__ "():" S2(__LINE__)

pool_t *pool_create(pool_t *parent, char *tag);
void pool_destroy(pool_t *p);
void *pool_alloc(pool_t *p, size_t bytes, char *tag);
void *pool_remember(pool_t *p, void *ptr, char *tag);
void register_cleanup(pool_t *p, void (*fn)(void *), void *data, char *tag);

int pool_debug_ctr();

#endif __POOL_H__