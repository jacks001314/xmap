/*
 * =====================================================================================
 *
 *       Filename:  xm_recv_task.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/08/2019 02:51:20 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jacksha (shajf), csp001314@163.com
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef XM_RECV_TASK_H
#define XM_RECV_TASK_H

typedef struct xm_recv_task_t xm_recv_task_t;

#include "xm_context.h"
#include "xm_task.h"
#include "xm_store.h"

struct xm_recv_task_t {

    xm_task_t task;

    uint8_t **seen;

	xm_store_t *xstore;

    xm_context_t *context;
	
    xm_if_driver_private_data_t *if_driver;

    int thread_id;
};

extern xm_recv_task_t * xm_recv_task_create(xm_context_t *context,int thread_id);

#endif /* XM_RECV_TASK_H */

