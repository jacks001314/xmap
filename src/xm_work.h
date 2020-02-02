/*
 * =====================================================================================
 *
 *       Filename:  xm_work.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/15/2019 10:59:37 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jacksha (shajf), csp001314@163.com
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef XM_WORK_H
#define XM_WORK_H

#include "xm_context.h"
#include "xm_tables.h"
#include "xm_core_pool.h"
#include "xm_send_task.h"
#include "xm_recv_task.h"

typedef struct xm_work_t xm_work_t;

struct xm_work_t {

    xm_context_t *context;

    xm_core_pool_t *cpool;

    xm_array_header_t *send_tasks;
    
    xm_recv_task_t* recv_task;

};

extern xm_work_t *xm_work_create(xm_context_t *context);

extern void xm_work_start(xm_work_t *work);

static inline void xm_work_add_send_task(xm_work_t *work,xm_send_task_t *send_task){

    *(xm_send_task_t**)xm_array_push(work->send_tasks) = send_task;
}


#endif /*XM_WORK_H*/
