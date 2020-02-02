/*
 * =====================================================================================
 *
 *       Filename:  xm_store.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/29/2019 11:58:59 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jacksha (shajf), csp001314@163.com
 *   Organization:  
 *
 * =====================================================================================
 */

#include "xm_store.h"
#include "xm_log.h"
#include "xm_shm_entry_record.h"
#include "xm_util.h"

xm_store_t *xm_store_create(xm_pool_t *mp,const char *shm_fname,
	uint64_t shm_fsize,uint64_t fentry_size,uint64_t shm_flush_timeout){

	xm_store_t *xstore = (xm_store_t*)xm_palloc(mp,sizeof(*xstore));

	xstore->mp  = mp;
	xstore->shm_flush_timeout =  shm_flush_timeout;
	xstore->shm_last_time = xm_get_current_timems()/1000;

	xstore->shm_fmt = xm_shm_format_with_mmap_create(mp,shm_fname,shm_fsize,fentry_size,0,1);

	if(xstore->shm_fmt == NULL){
	
		xm_log(XM_LOG_ERR,"Cannot Create shm store failed for  xstore!");
		return NULL;
	}
	

    msgpack_sbuffer_init(&xstore->pk_buf);
    msgpack_packer_init(&xstore->pk,&xstore->pk_buf,msgpack_sbuffer_write);


	return xstore;

}

void xm_store_destroy(xm_store_t *xstore){

    msgpack_sbuffer_destroy(&xstore->pk_buf);

}

static void _xstore_shm_flush(xm_store_t *xstore){

	uint64_t cur_time = xm_get_current_timems()/1000;

	if(cur_time-xstore->shm_last_time>xstore->shm_flush_timeout){

		xm_shm_format_flush(xstore->shm_fmt);

		xstore->shm_last_time = cur_time;
	}
}

int xm_store_flush(xm_store_t *xstore,uint8_t type) {

	void *data;
	size_t dlen;

    data = xstore->pk_buf.data;
    dlen = xstore->pk_buf.size;


	xm_shm_entry_record_t shm_entry_rcd;
	shm_entry_rcd.type = type;

	xm_shm_entry_record_put(xstore->shm_fmt,&shm_entry_rcd,data,dlen);

    msgpack_sbuffer_clear(&xstore->pk_buf);

	_xstore_shm_flush(xstore);

	/*ok*/
	return 0;

}

