/*
 *
 *      Filename: xm_core_pool.h
 *
 *        Author: shajf,csp001314@163.com
 *   Description: ---
 *        Create: 2017-01-13 17:05:23
 * Last Modified: 2017-01-13 17:05:23
 */

#ifndef XM_CORE_POOL_H
#define XM_CORE_POOL_H

typedef struct xm_core_pool_t xm_core_pool_t;
typedef struct xm_core_t xm_core_t;

#include <sched.h>
#include "xm_task.h"
#include "xm_constants.h"
#include "xm_mpool.h"

enum core_state {

	WAITING,
	RUNNING,
	FINISHED,
};

#define XM_CORE_MAX 256

struct xm_core_t {

	xm_core_pool_t *cpool;
	xm_task_t *tsk;
	uint32_t core_id;

	uint32_t socket;
	uint32_t core_idx;

	int state;

	int enabled;

	cpu_set_t cpuset;

	pthread_t thread_id;

	int pipe_master2slave[2];
	int pipe_slave2master[2];
};

struct xm_core_pool_t {

	xm_pool_t *mp;

	xm_core_t * cores[XM_CORE_MAX];
	
	uint32_t config_core_count;

	xm_core_t *master_core;

	uint32_t sys_core_count;
	

};

extern xm_core_pool_t * xm_core_pool_create(xm_pool_t *mp,const char *core_mask);

extern int xm_core_pool_bind_task(xm_core_pool_t *cpool,xm_task_t *tsk);

extern int xm_core_pool_slaves_setup(xm_core_pool_t *cpool);

extern void xm_core_pool_wait_for_slaves(xm_core_pool_t *cpool);

static inline xm_core_t * 
xm_next_core_get(xm_core_pool_t *cpool,int skip_master,int idx){

	xm_core_t *core = NULL;

	idx++;

	for(;idx<(int)cpool->sys_core_count;idx++){
	
		core = cpool->cores[idx];
		if(((core == cpool->master_core)&&skip_master)||(core->enabled == 0))
			continue;

		return core;
	}

	return NULL;
}

/**
 * Macro to browse all running lcores.
 */
#define CPOOL_CORE_FOREACH(cpool,core)						\
	for (core = xm_next_core_get(cpool, 0, -1);				\
	     core!=NULL;										\
	     core = xm_next_core_get(cpool, 0, core->core_idx))

/**
 * Macro to browse all running lcores except the master lcore.
 */

#define CPOOL_CORE_FOREACH_SLAVE(cpool,core)				\
	for (core = xm_next_core_get(cpool, 1, -1);				\
	     core!=NULL;										\
	     core = xm_next_core_get(cpool, 1, core->core_idx))


#endif /* XM_CORE_POOL_H */
