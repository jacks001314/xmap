/*
 * =====================================================================================
 *
 *       Filename:  xm_if_pfring.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/26/2019 12:11:24 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jacksha (shajf), csp001314@163.com
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef XM_IF_PFRING_H
#define XM_IF_PFRING_H

typedef struct xm_if_pfring_t xm_if_pfring_t;
typedef struct xm_if_pfring_private_data_t xm_if_pfring_private_data_t;
typedef struct xm_if_pfring_context_t xm_if_pfring_context_t;

#include "xm_mpool.h"
#include "xm_if.h"
#include <pfring_zc.h>

struct xm_if_pfring_t {

	xm_if_driver_t driver;

	xm_pool_t *mp;

	xm_if_pfring_context_t *pcontext;

	pfring_zc_cluster *zc;

	pfring_zc_queue *pf_send;
	
	pfring_zc_queue *pf_rcv;

	pfring_zc_buffer_pool *prefetches;

	size_t total_buf_num;
};

struct xm_if_pfring_private_data_t {

	xm_if_driver_private_data_t drv_data;

	pfring_zc_pkt_buff **send_bufs;

	pfring_zc_queue *send_zq;

	pfring_zc_queue *rcv_zq;

	pfring_zc_pkt_buff **rcv_buf;

};

struct xm_if_pfring_context_t {

	/* The number of receive queue */
	size_t rx_n_queue;
	
	/* The number of transmit queue */
	size_t tx_n_queue;

	/* The number of buffer per a receive queue */
	size_t rx_per_n_buf;

	/* The number of buffer per a transmit queue */
	size_t tx_per_n_buf;

	/*The one receive buffer size */
	size_t rx_buf_size;

	/* The one transmit buffer size */
	size_t tx_buf_size;

	/* The rx queue len */
	size_t rx_queue_len;

	/* The tx queue len */
	size_t tx_queue_len;

	size_t max_card_slots;

	/* The cluster id */
	uint32_t cluster_id;

	/*hugepages mountpoint */
	const char *mnt_dir;

	int use_zc;
};

static inline size_t xm_if_pfring_total_buffers(xm_if_pfring_t *drv){

	xm_if_pfring_context_t *pcontext = drv->pcontext;

	size_t send_buffer_num = pcontext->tx_n_queue*pcontext->tx_per_n_buf+pcontext->tx_n_queue*pcontext->tx_queue_len;

	size_t rcv_num = pcontext->rx_n_queue*pcontext->rx_per_n_buf;


	return (send_buffer_num+pcontext->max_card_slots*2+rcv_num+2);
}


extern xm_if_driver_t * xm_if_pfring_create(xm_pool_t *mp,xm_if_t *interface);

extern xm_if_pfring_context_t *xm_if_pfring_context_create(xm_pool_t *mp,const char *cfname);

#endif /*XM_IF_PFRING_H*/

