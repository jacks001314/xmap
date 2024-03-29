/*
 *
 *      Filename: xm_buffer.h
 *
 *        Author: shajf,csp001314@gmail.com
 *   Description: ---
 *        Create: 2018-04-10 18:34:50
 * Last Modified: 2018-04-10 18:34:50
 */

#ifndef XM_BUFFER_H
#define XM_BUFFER_H

typedef struct xm_buffer_t xm_buffer_t;

#include <stdlib.h>

struct xm_buffer_t {


	void *bdata;

	size_t b_current_size;

};

#define B_SIZE_DEFAULT 64*1024
#define B_SIZE_EXT 32

static inline int xm_buffer_init(xm_buffer_t *bf){


	bf->bdata = malloc(B_SIZE_DEFAULT+B_SIZE_EXT);
	if(bf->bdata == NULL)
		return -1;

	bf->b_current_size = B_SIZE_DEFAULT;

	return 0;
}

static inline int xm_buffer_init2(xm_buffer_t *bf,size_t bsize){

	if(bsize == 0)
		bsize = B_SIZE_DEFAULT;

	bf->bdata = malloc(bsize+B_SIZE_EXT);
	if(bf->bdata == NULL)
		return -1;

	bf->b_current_size = bsize;

	return 0;
}

#define xm_buffer_need_update(bf,nsize) ((bf)->b_current_size<nsize) 

static inline int xm_buffer_update(xm_buffer_t *bf,size_t nsize) {


	/*Free old buffer*/
	free(bf->bdata);

	bf->bdata = NULL;
	bf->b_current_size = 0;

	bf->bdata = malloc(nsize+B_SIZE_EXT);
	if(bf->bdata == NULL)
		return -1;

	bf->b_current_size = nsize;

	return 0;
}

#endif /*XM_BUFFER_H*/
