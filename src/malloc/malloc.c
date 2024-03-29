#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "../internal/internal.h"
#include <stdint.h>
#include <sys/mman.h>
#include <errno.h>
#include <limits.h>

/*
	The ieee-libc malloc uses a table chain allocator which is accessed through
	a series of divisions which reduce the input to a granulated range. New
	magazines are dynamically allocated. There is no superstructure, instead
	the free list is used to track all allocations and objects are not added 
	until free() time.
*/


/* shoot for a 64 unit granularity ~[512, 32768) as (32768/512 = 64) */
#define TWOTO32 4294967296			/* 2^32 */
#define FOLDSIZE TWOTO32 / (256*512*64)		/* ~2^32 / 2^23 = ~512 */
#define CHAINLEN TWOTO32 / (256*512*128)	/* ~2^32 / 2^24 = ~256 */
#define SLOT TWOTO32 / (FOLDSIZE*CHAINLEN)	/* ~2^32 / (~256*~512) = ~32768 */
#define FOLD TWOTO32 / CHAINLEN			/* ~2^32 / ~256 = ~16777216 */


typedef struct object
{
	size_t size;
} object;

typedef struct flist
{
	struct flist *next;
	struct flist *prev;
	object *node;
}flist;

typedef struct chain
{ 
	flist **magazine;
	flist **head;
}chain;

static chain **tchain; 

static const size_t chunk_size = 4096;

static size_t magno(size_t i)
{
	return i / FOLD;
}

static size_t bulletno(size_t i)
{
	return i / SLOT;
}

static void initmag(size_t i)
{
	size_t z = magno(i);
	chain *c = NULL;

	if (!tchain)
		tchain = __mmap_inter(sizeof (chain) * CHAINLEN);

	if (!(c = tchain[z])) {
		c = __mmap_inter(sizeof (chain));
	}

	if (!c->magazine)
	{
		c->magazine = __mmap_inter(sizeof (flist) * FOLDSIZE);
		c->head = __mmap_inter(sizeof (flist) * FOLDSIZE);
	}
	tchain[z] = c;
}

static flist *delmiddle(flist *o)
{
	flist *tmp = o->prev;
	o->prev->next = o->next;
	o->next->prev = o->prev;
	munmap(o, sizeof(flist));
	return tmp;
}

static int addfreenode(object *node)
{
	size_t bullet = bulletno(node->size);
	size_t mag = magno(node->size);
	flist *o = NULL;
	chain *c = tchain[mag];
	flist *last = c->head[bullet]; 

	if (!(o = __mmap_inter(sizeof (flist)))) {
		return 1;
	}

	if (last) {
		last->next = o;
	}
	o->next = NULL;
	o->prev = last;
	o->node = node; 
	c->head[bullet] = o;
	if (!(c->magazine[bullet])) { 
		 c->magazine[bullet] = o;
	}
	tchain[mag] = c;
	return 0;
}

static object *findfree(size_t size)
{
	object *t = NULL;
	object *ret = NULL;
	flist *o = NULL;
	chain *c = NULL;

	size_t i = bulletno(size);
	size_t z = magno(size);
	
	for (; z < CHAINLEN; ++z) {
		c = tchain[z];
		for (; c && i < FOLDSIZE; ++i) {
			for (o = c->magazine[i]; o ; o = o->next) {
				t = o->node;
				if (t == NULL || o == c->head[i] || o == c->magazine[i])
					continue;
				if (t->size >= size && ret == NULL) {
					o = delmiddle(o);
					ret = t;
					goto end;
				}else {
					o = delmiddle(o);
					munmap(t, t->size + sizeof (object));
					t = NULL;
				}
			}
		}
		i = 0;
	}
	end:
	return ret;
}

static object *morecore(size_t size)
{
	object *o = NULL;
	size_t t = 0;

	if (__safe_uadd_sz(size, sizeof (object), &t, SIZE_MAX) == -1) {
		goto error;
	}

	if (!(o = __mmap_inter(t))) {
		goto error;
	}
	o->size = size;
	return o;

	error:
	errno = ENOMEM;
	return NULL;
}

void *malloc(size_t size)
{
	size_t mul = 1;
	object *o = NULL;
	if (size >= TWOTO32) {
		goto core;
	}

        if (size > chunk_size) {
                mul += (size / chunk_size);
	}

        if (!((size_t)-1 / chunk_size < mul)) {
                size = (chunk_size * mul);
	}

	initmag(size);
	
	if (!(o = findfree(size))) {
		core:
		if (!(o = morecore(size))) {
			return NULL;
		}
	}

	return (o + 1);
}

void free(void *ptr)
{
	object *o = NULL;
	if (!ptr) {
		return;
	}
	o = (object *)ptr - 1;
	if (o->size >= TWOTO32) {
		munmap(o, o->size);
		return;
	}
	addfreenode(o);
}

void *realloc(void *ptr, size_t size)
{
	void *ret = NULL;
	object *o = NULL;
	if (!ptr) {
		return malloc(size);
	}

	o = (object *)ptr - 1;
	if (o->size >= size) {
		return ptr;
	}

	if (!(ret = malloc(size))) {
		return NULL;
	}

	memcpy(ret, ptr, o->size);
	free(ptr);
	return ret;
}

void *calloc(size_t nmemb, size_t size)
{
	void *o = NULL;
	size_t t = 0;
	if(__safe_umul_sz(nmemb, size, &t, (size_t)-1) == -1) { 
		return NULL;
	}
	if (!(o = malloc(t))) {
		return NULL;
	}
	memset(o, 0, t);
	return o;
}

