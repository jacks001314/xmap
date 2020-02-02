/*
 * =====================================================================================
 *
 *       Filename:  xm_if.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/26/2019 10:38:22 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jacksha (shajf), csp001314@163.com
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef XM_IF_H
#define XM_IF_H

typedef struct xm_if_t xm_if_t;
typedef struct xm_if_driver_t xm_if_driver_t;
typedef struct xm_if_driver_private_data_t xm_if_driver_private_data_t;

#include "xm_mpool.h"
#include "xm_tables.h"
#include "xm_headers.h"

#define IF_DPDK 0
#define IF_PFRING 1
#define IF_LIBPCAP 2
#define IF_LINUX 3

#define IF_DPDK_NAME "dpdk"
#define IF_PFRING_NAME "pfring"
#define IF_LIBPCAP_NAME "libpcap"
#define IF_LINUX_NAME "linux"
#define MAC_ADDR_LEN_BYTES 6

struct xm_if_t {

    xm_pool_t *mp;
    
    const char *if_name;
    
    unsigned char if_mac[MAC_ADDR_LEN_BYTES];

    const char *if_cfname;

    int if_index;
    int if_type;

    struct in_addr if_addr;

    xm_if_driver_t *if_driver;
};

struct xm_if_driver_t {

    xm_pool_t *mp;
    xm_array_header_t *drv_datas;
    xm_if_t *interface;

	int (*init)(xm_if_driver_t *drv,void *priv_data);   /* init driver */

	int (*start)(xm_if_driver_t *drv,void *priv_data); /* start a driver,prepare to receive or send packets */

	xm_if_driver_private_data_t * (*create_rx_private_data)(xm_if_driver_t *drv,int socket,void *priv_data);
	
	xm_if_driver_private_data_t * (*create_tx_private_data)(xm_if_driver_t *drv,int socket,void *priv_data);

	int (*stop)(xm_if_driver_t *drv,void *priv_data); /* stop a driver,stop receive or send packets */

	void (*fin)(xm_if_driver_t *drv,void *priv_data); /* finish a driver,cleanup some resources */
};

struct xm_if_driver_private_data_t {

    xm_if_driver_t *drv;
    xm_pool_t *mp;

	/* receive the packets into packet buffer */ 
	size_t (*receive)(xm_if_driver_private_data_t *drv_data,
		int (*packet_handle)(xm_if_driver_private_data_t *pdrv_data,void *pkt_data,size_t pkt_len,void *pkt_buf,void *priv_data),
		void *priv_data);

	/* free pkt buf */
	void (*pkt_free)(xm_if_driver_private_data_t *drv_data,void *pkt_buf,void *priv_data);

	/* send packets into driver */
	int (*send)(xm_if_driver_private_data_t *drv_data,void *pkt_data,size_t pkt_len,size_t idx,void *priv_data); 
};

extern xm_if_t *xm_if_create(xm_pool_t *mp,const char *if_name,const char *if_type,const char *if_cfname);

static inline void xm_if_driver_private_data_add(xm_if_driver_t *drv,xm_if_driver_private_data_t *drv_data){

	*(xm_if_driver_private_data_t**)xm_array_push(drv->drv_datas) = drv_data;
}

#endif /* XM_IF_H */

