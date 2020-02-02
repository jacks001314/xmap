/*
 * =====================================================================================
 *
 *       Filename:  xm_shm_interface.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  10/31/2017 05:32:44 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  shajf (jacks), csp001314@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef XM_SHM_INTERFACE_H
#define XM_SHM_INTERFACE_H

typedef struct xm_shm_interface_t xm_shm_interface_t;
typedef struct xm_shm_header_t xm_shm_header_t;
typedef struct xm_shm_entry_t xm_shm_entry_t;

#include "xm_mpool.h"

struct xm_shm_interface_t {


   xm_pool_t *mp;

   uint64_t page_size;

   uint64_t size;

   unsigned is_write:1;

   xm_shm_header_t *shm_header;

   void* (*shm_address_get)(xm_shm_interface_t *shm_int,uint64_t offset);

   void (*shm_address_free)(xm_shm_interface_t *shm_int,void *address);

};

#pragma pack(push,1)
struct xm_shm_header_t {
	uint64_t shm_entries_start;
	uint64_t shm_entries_count;
	uint64_t shm_entry_size;
	uint64_t shm_entries_count_cur;
	uint64_t shm_write_entry_pos;
	uint64_t shm_read_entry_pos;
	uint64_t shm_priv_data_size;
    uint64_t shm_last_write_time;
    uint64_t shm_last_read_time;
    uint64_t shm_last_write_ok_time;
    uint64_t shm_last_read_ok_time;
};
#pragma pack(pop)

struct xm_shm_entry_t {
	xm_shm_interface_t *shm_int;
	uint64_t entry_index;
	void *start;
	void *pos;
	void *end;
};

static inline void * xm_shm_priv_data_address(xm_shm_interface_t *shm_int){

	xm_shm_header_t *mh = shm_int->shm_header;

	if(mh->shm_priv_data_size == 0)
		return NULL;

	return (void*)(mh+1);
}

static inline uint64_t xm_shm_entry_pos_offset(xm_shm_interface_t *shm_int,uint64_t indx){

    return shm_int->shm_header->shm_entry_size*indx+shm_int->shm_header->shm_entries_start;
}

static inline void xm_shm_entry_pos_update(xm_shm_interface_t *shm_int,int pos_c,int c){

    uint64_t * pos = shm_int->is_write?&shm_int->shm_header->shm_write_entry_pos:&shm_int->shm_header->shm_read_entry_pos;
    uint64_t count = shm_int->shm_header->shm_entries_count;

    *pos = (*pos+pos_c)%count;
    shm_int->shm_header->shm_entries_count_cur += c;

}

static inline int xm_shm_empty(xm_shm_interface_t *shm_int){
    return shm_int->shm_header->shm_entries_count_cur == 0;
}

static inline int xm_shm_full(xm_shm_interface_t *shm_int){
    return shm_int->shm_header->shm_entries_count_cur == shm_int->shm_header->shm_entries_count;
}


extern void xm_shm_init(xm_shm_interface_t *shm_int,void *address,
  uint64_t size,uint64_t entry_size,uint64_t priv_data_size,uint64_t page_size,int is_write);

extern int xm_shm_entry_get(xm_shm_interface_t *shm_int,xm_shm_entry_t *entry);

extern void xm_shm_entry_commit(xm_shm_interface_t *shm_int,xm_shm_entry_t *entry);

#endif /*XM_SHM_INTERFACE_H*/
