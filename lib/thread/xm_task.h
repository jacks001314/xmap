/*
 *
 *      Filename: xm_task.h
 *
 *        Author: shajf,csp001314@163.com
 *   Description: ---
 *        Create: 2017-01-13 18:55:34
 * Last Modified: 2017-01-13 18:55:34
 */

#ifndef XM_TASK_H
#define XM_TASK_H

typedef struct xm_task_t xm_task_t;

#include "xm_constants.h"
#include "xm_core_pool.h"

#define TASK_RETURN_OK 0
#define TASK_RETURN_EXIT -1
#define TASK_RETURN_SLEEP -2

struct xm_task_t {

    xm_core_t *core;
    void *priv_data;
    uint32_t tsk_id;

    int (*init)(xm_task_t *task,void *priv_data);
    int (*run)(xm_task_t *task,void *priv_data);
    int (*exit)(xm_task_t *task,void *priv_data);
};

extern int  xm_task_run(xm_task_t *tsk);


#endif /* XM_TASK_H */
