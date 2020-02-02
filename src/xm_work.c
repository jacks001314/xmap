/*
 * =====================================================================================
 *
 *       Filename:  xm_work.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/15/2019 11:33:52 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jacksha (shajf), csp001314@163.com
 *   Organization:  
 *
 * =====================================================================================
 */
#include "xm_work.h"
#include "xm_log.h"

xm_work_t *xm_work_create(xm_context_t *context){

    int i;
    xm_send_task_t *send_task;

    xm_work_t *work = (xm_work_t*)xm_palloc(context->mp,sizeof(*work));

    work->context = context;
    work->cpool = xm_core_pool_create(context->mp,context->core_mask);
    if(work->cpool == NULL){

        xm_log(XM_LOG_ERR,"create core pool failed for work!");
        return NULL;
    }

    work->send_tasks = xm_array_make(context->mp,16,sizeof(xm_send_task_t*));

    if(work->send_tasks == NULL){

        xm_log(XM_LOG_ERR,"Create array failed!");
        return NULL;
    }

    for(i = 0;i<context->senders;i++){

        send_task = xm_send_task_create(context,i+1);
        if(send_task == NULL){

            xm_log(XM_LOG_ERR,"Create send task:%d failed!",i+1);
            return NULL;
        }
        if(xm_core_pool_bind_task(work->cpool,(xm_task_t*)send_task)){

            xm_log(XM_LOG_ERR,"Cannot bind send task:%d to a core!",i+1);
            return NULL;
        }

        xm_work_add_send_task(work,send_task);
    }

    work->recv_task = xm_recv_task_create(context,1);
    if(work->recv_task == NULL){

        xm_log(XM_LOG_ERR,"Create recv task failed!");
        return NULL;
    }
    
    if(xm_core_pool_bind_task(work->cpool,(xm_task_t*)work->recv_task)){

        xm_log(XM_LOG_ERR,"Cannot bind recv task:%d to a core!",1);
        return NULL;
    }


    return work;
}

void xm_work_start(xm_work_t *work){

	if(xm_core_pool_slaves_setup(work->cpool)){
 
         xm_log(XM_LOG_ERR,"setup all slave threads failed!\n");
         exit(-1);
     }
 
     xm_core_pool_wait_for_slaves(work->cpool);
 
}

