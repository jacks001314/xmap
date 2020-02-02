/*
 * =====================================================================================
 *
 *       Filename:  xm_store.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/29/2019 11:01:05 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jacksha (shajf), csp001314@163.com
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef XM_STORE_H
#define XM_STORE_H


typedef struct xm_store_t xm_store_t;

#include <msgpack.h>
#include "xm_shm_format.h"
#include "xm_mpool.h"

struct xm_store_t {

    xm_pool_t *mp;
    xm_shm_format_t *shm_fmt;

    msgpack_packer pk;
    msgpack_sbuffer pk_buf;

    uint64_t shm_last_time;
    uint64_t shm_flush_timeout;

};

extern xm_store_t *xm_store_create(xm_pool_t *mp,const char *shm_fname,
	uint64_t shm_fsize,uint64_t fentry_size,uint64_t shm_flush_timeout);


extern void xm_store_destroy(xm_store_t *xstore);

extern int xm_store_flush(xm_store_t *xstore,uint8_t type);

static inline int _str_empty(const char *v){

    return v == NULL || strlen(v) == 0;
}

static inline void xm_store_write_str(xm_store_t *xstore,const char *str){

    size_t len = strlen(str);
    msgpack_pack_str(&xstore->pk,len);
    msgpack_pack_str_body(&xstore->pk,str,len);
}

static inline void xm_store_write_bin(xm_store_t *xstore,void *data,size_t dlen){

    msgpack_pack_bin(&xstore->pk,dlen);
    msgpack_pack_bin_body(&xstore->pk,data,dlen);
}

static inline void xm_store_write_bin_kv(xm_store_t *xstore,const char *k,void *data,size_t dlen){
    
	if(_str_empty(k)==0){
		xm_store_write_str(xstore,k);
	}
	
	xm_store_write_bin(xstore,data,dlen);

}

static inline void xm_store_write_kv(xm_store_t *xstore,const char *k,const char *v){
    
    xm_store_write_str(xstore,k);
    xm_store_write_str(xstore,v==NULL?"":v);
}

static inline void xm_store_write_number_str(xm_store_t *xstore,const char *k,unsigned long number){                                               
                                                                                                                                       
    char b[128]={0};                                                                                                                   
    snprintf(b,127,"%lu",number);                                                                                                      
    xm_store_write_kv(xstore,k,(const char *)b);                                                                                            
}                                                                                                                                      



static inline void xm_store_map_start(xm_store_t *xstore,const char *k,size_t n){

    if(_str_empty(k)==0){
        xm_store_write_str(xstore,k); 
    }
    msgpack_pack_map(&xstore->pk,n);
}

static inline void xm_store_write_uint(xm_store_t *xstore,const char *k,unsigned int v){
    
	if(_str_empty(k)==0){
		xm_store_write_str(xstore,k);
	}
	
	msgpack_pack_unsigned_int(&xstore->pk,v);
}

static inline void xm_store_write_uint8(xm_store_t *xstore,const char *k,uint8_t v){
    
	if(_str_empty(k)==0){
		xm_store_write_str(xstore,k);
	}
	
	msgpack_pack_uint8(&xstore->pk,v);
}

static inline void xm_store_write_uint16(xm_store_t *xstore,const char *k,uint16_t v){
    
	if(_str_empty(k)==0){
		xm_store_write_str(xstore,k);
	}
	
	msgpack_pack_uint16(&xstore->pk,v);
}

static inline void xm_store_write_uint32(xm_store_t *xstore,const char *k,uint32_t v){
    
	if(_str_empty(k)==0){
		xm_store_write_str(xstore,k);
	}
	
	msgpack_pack_uint32(&xstore->pk,v);
}

static inline void xm_store_write_uint64(xm_store_t *xstore,const char *k,uint64_t v){
    
	if(_str_empty(k)==0){
		xm_store_write_str(xstore,k);
	}
	
	msgpack_pack_uint64(&xstore->pk,v);
}

static inline void xm_store_array_start(xm_store_t *xstore,const char *k,size_t n){

    if(_str_empty(k)==0){
        xm_store_write_str(xstore,k); 
    }
    msgpack_pack_array(&xstore->pk,n);
}


#endif /* XM_STORE_H */

