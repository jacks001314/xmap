/*
 * =====================================================================================
 *
 *       Filename:  xm_send_task.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/31/2019 02:34:37 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jacksha (shajf), csp001314@163.com
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef XM_SEND_TASK_H
#define XM_SEND_TASK_H

typedef struct xm_send_task_t xm_send_task_t;

#include "xm_shard.h"
#include "xm_context.h"
#include "xm_task.h"

struct xm_send_task_t {

    xm_task_t task;

    xm_context_t *context;

    xm_shard_t *shard;

    xm_if_driver_private_data_t *if_driver;

	int thread_id;
};


extern xm_send_task_t * xm_send_task_create(xm_context_t *context,int thread_id);

#endif /*XM_SEND_TASK_H*/

