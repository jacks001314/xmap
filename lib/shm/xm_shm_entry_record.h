/*
 * =====================================================================================
 *
 *       Filename:  xm_shm_entry_record.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/29/2019 11:26:13 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jacksha (shajf), csp001314@163.com
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef XM_SHM_ENTRY_RECORD_H
#define XM_SHM_ENTRY_RECORD_H

typedef struct xm_shm_entry_record_t xm_shm_entry_record_t;

#include "xm_shm_format.h"

/*Define record types*/
#define RECORD_TYPE_TCP_SYNSCAN 1

struct xm_shm_entry_record_t {

    xm_shm_record_t shm_rcd;
    uint8_t type;
};

#define XM_SHM_ENTRY_RECORD_HEADER_SIZE (sizeof(uint8_t)+\
	sizeof(uint32_t)*3+\
	sizeof(size_t))

extern xm_shm_format_t * xm_shm_format_with_mmap_create(xm_pool_t *mp,const char *fname,uint64_t fsize,uint64_t entry_size,
	uint64_t priv_data_size,int is_write);


extern xm_shm_format_t * xm_shm_format_with_shm_create(xm_pool_t *mp,const char *key,int proj_id,uint64_t size,
	uint64_t entry_size,uint64_t priv_data_size,int is_write);

extern int xm_shm_entry_record_put(xm_shm_format_t *shm_fmt,xm_shm_entry_record_t *shm_entry_rcd,void *data,size_t dlen);


#endif /* XM_SHM_ENTRY_RECORD_H */
