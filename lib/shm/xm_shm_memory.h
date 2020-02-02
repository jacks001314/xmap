/*
 * =====================================================================================
 *
 *       Filename:  xm_shm_memory.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  10/31/2017 10:50:54 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  shajf (jacks), csp001314@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef XM_SHM_MEMORY_H
#define XM_SHM_MEMORY_H

typedef struct xm_shm_memory_t xm_shm_memory_t;

#include "xm_shm_interface.h"

struct xm_shm_memory_t {

   xm_shm_interface_t shm_int;

   void *start_address;

   const char *key;
   int shm_id;
   int proj_id;
};

xm_shm_interface_t * xm_shm_memory_create(xm_pool_t *mp,const char *key,int proj_id,uint64_t size,uint64_t  entry_size,uint64_t priv_data_size,int is_write);


extern void xm_shm_memory_destroy(xm_shm_interface_t *shm_int);

#endif /* XM_SHM_MEMORY_H */

