/*
 * =====================================================================================
 *
 *       Filename:  xm_shm_entry_record.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/29/2019 11:37:13 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jacksha (shajf), csp001314@163.com
 *   Organization:  
 *
 * =====================================================================================
 */
#include "xm_shm_entry_record.h"

#include "xm_log.h"
#include "xm_shm_mmap.h"
#include "xm_shm_memory.h"


static void _shm_entry_record_write(xm_shm_format_t *fmt,xm_shm_record_t *rcd){

    xm_shm_entry_header_t *header = &fmt->cur_header;
    xm_bin_format_t *bfmt = &header->bfmt;

	xm_shm_entry_record_t *shm_entry_rcd = (xm_shm_entry_record_t*)rcd;

    xm_bf_uint8_write(bfmt,shm_entry_rcd->type);

}

static int  _shm_entry_record_read(xm_shm_entry_iterator_t *iter,xm_bin_format_t *bfmt,xm_shm_record_t *shm_record){

	xm_shm_entry_record_t *shm_entry_rcd = (xm_shm_entry_record_t*)shm_record;

    shm_entry_rcd->type = xm_bf_uint8_read(bfmt); 

	return 0;
}


xm_shm_format_t * xm_shm_format_with_mmap_create(xm_pool_t *mp,const char *fname,uint64_t fsize,uint64_t entry_size,
	uint64_t priv_data_size,int is_write){

	xm_shm_interface_t *shm_int = NULL;
	xm_shm_entry_record_t *shm_entry_rcd = (xm_shm_entry_record_t*)xm_pcalloc(mp,sizeof(*shm_entry_rcd));

	shm_int = xm_shm_mmap_create(mp,fname,fsize,entry_size,priv_data_size,is_write);
	if(shm_int == NULL){
	
		xm_log(XM_LOG_ERR,"Create a mmap file failed for packet!");
		return NULL;
	}

	return xm_shm_format_create(mp,shm_int,entry_size,
		XM_SHM_ENTRY_RECORD_HEADER_SIZE,
		(xm_shm_record_t*)shm_entry_rcd,
		_shm_entry_record_write,
		_shm_entry_record_read);

}

xm_shm_format_t * xm_shm_format_with_shm_create(xm_pool_t *mp,const char *key,int proj_id,uint64_t size,
	uint64_t entry_size,uint64_t priv_data_size,int is_write){

	xm_shm_entry_record_t *shm_entry_rcd = (xm_shm_entry_record_t*)xm_pcalloc(mp,sizeof(*shm_entry_rcd));
	xm_shm_interface_t *shm_int = NULL;
	
	shm_int = xm_shm_memory_create(mp,key,proj_id,size,entry_size,priv_data_size,is_write);
	if(shm_int == NULL){
	
		xm_log(XM_LOG_ERR,"Create a shm memory failed for packet!");
		return NULL;
	}

	return xm_shm_format_create(mp,shm_int,entry_size,
		XM_SHM_ENTRY_RECORD_HEADER_SIZE,
		(xm_shm_record_t*)shm_entry_rcd,
		_shm_entry_record_write,
		_shm_entry_record_read);

}

int xm_shm_entry_record_put(xm_shm_format_t *shm_fmt,xm_shm_entry_record_t *shm_entry_rcd,void *data,size_t dlen){

	int rc;

	xm_shm_record_t *shm_rcd = &shm_entry_rcd->shm_rcd;

	shm_rcd->magic = EIMAGIC;

	shm_rcd->record_size = XM_SHM_ENTRY_RECORD_HEADER_SIZE+dlen;

	shm_rcd->data_offset = 0;

	shm_rcd->data_len = dlen;
	shm_rcd->data = data;

    for(;;){

        rc = xm_shm_format_put(shm_fmt,(xm_shm_record_t*)shm_entry_rcd);
        if(rc == 0){
            return 0;
        }

        if(rc == -2){

            xm_log(XM_LOG_ERR,"The data is too large:%lu,discard it!",dlen);

            return -2;
        }

        xm_log(XM_LOG_WARN,"The shm is full,sleep and try again!");

        usleep(5);
    }

	return 0;

}

