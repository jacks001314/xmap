#include "xm_xalloc.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static void make_error(void) { printf("Out of memory\n"); }

void *xcalloc(size_t count, size_t size)
{
	void *res = calloc(count, size);
	if (res == NULL) {
		make_error();
	}
	return res;
}

void xfree(void *ptr) { free(ptr); }

void *xmalloc(size_t size)
{
	void *res = malloc(size);
	if (res == NULL) {
		make_error();
	}
	memset(res, 0, size);
	return res;
}

void *xrealloc(void *ptr, size_t size)
{
	void *res = realloc(ptr, size);
	if (res == NULL) {
		make_error();
	}
	return res;
}

