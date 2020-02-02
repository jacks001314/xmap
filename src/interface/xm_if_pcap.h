/*
 * =====================================================================================
 *
 *       Filename:  xm_if_pcap.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/26/2019 03:12:17 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jacksha (shajf), csp001314@163.com
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef XM_IF_PCAP_H
#define XM_IF_PCAP_H

typedef struct xm_if_pcap_t xm_if_pcap_t;
typedef struct xm_if_pcap_private_data_t xm_if_pcap_private_data_t;
typedef struct xm_if_pcap_context_t xm_if_pcap_context_t;

#include <pcap.h>
#include "xm_mpool.h"
#include "xm_if.h"

struct xm_if_pcap_t {

	xm_if_driver_t driver;
	xm_pool_t *mp;

	xm_if_pcap_context_t *pcontext;

	pcap_t *pcap;
};

struct xm_if_pcap_private_data_t {

	xm_if_driver_private_data_t drv_data;
};

struct xm_if_pcap_context_t {

	size_t snaplen;

	uint64_t wait_ms;

	int promisc_mode;

	const char *filter_text;
};

extern xm_if_pcap_context_t * xm_if_pcap_context_create(xm_pool_t *mp,const char *cfname);

extern xm_if_driver_t * xm_if_pcap_create(xm_pool_t *mp,xm_if_t *interface);

#endif /* XM_IF_PCAP_H */
