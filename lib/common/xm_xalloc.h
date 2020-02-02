#ifndef XM_ALLOC_H
#define XM_ALLOC_H

#include <stddef.h>

void *xcalloc(size_t count, size_t size);

void xfree(void *ptr);

void *xmalloc(size_t size);

void *xrealloc(void *ptr, size_t size);

#endif /* XM_ALLOC_H */
