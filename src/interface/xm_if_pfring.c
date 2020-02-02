/*
 * =====================================================================================
 *
 *       Filename:  xm_if_pfring.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/26/2019 01:25:31 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jacksha (shajf), csp001314@163.com
 *   Organization:  
 *
 * =====================================================================================
 */
#include "xm_if_pfring.h"
#include "xm_log.h"
#include "xm_constants.h"

static int _pfring_init(xm_if_driver_t *driver,void *priv_data xm_unused){

	xm_if_pfring_t *pdriver = (xm_if_pfring_t*)driver;
	xm_if_pfring_context_t *pcontext = pdriver->pcontext;

	size_t max_packet_size = XM_MAX(pcontext->rx_buf_size,pcontext->tx_buf_size);

	pdriver->zc = pfring_zc_create_cluster(pcontext->cluster_id,
		max_packet_size,
		0,
		pdriver->total_buf_num,
		0,
		pcontext->mnt_dir); 

	if(pdriver->zc == NULL){
	
		xm_log(XM_LOG_ERR,"create pfring cluster failed!");
		return -1;
	}

	pdriver->pf_rcv = pfring_zc_open_device(pdriver->zc,
		driver->interface->if_name,
		rx_only,0);
	
	pdriver->pf_send = pfring_zc_open_device(pdriver->zc,
		driver->interface->if_name,
		tx_only,0);

	if(pdriver->pf_send == NULL||pdriver->pf_rcv == NULL){
	
		xm_log(XM_LOG_ERR,"open pfing interface:%s to send packet failed! ",
			driver->interface->if_name);

		return -1;
	}

	/* ok */
	return 0;
}

static int _pfring_start(xm_if_driver_t *driver xm_unused,void *priv_data  xm_unused){

	/* ok */
	return 0;
}

static int _pfring_stop(xm_if_driver_t *driver xm_unused,void *priv_data xm_unused){


	return 0;
}

static size_t _pfring_rcv(xm_if_driver_private_data_t *pdrv_data,
	int (*packet_handle)(xm_if_driver_private_data_t *pdrv_data,void *pkt_data,size_t pkt_len,void *pkt_buf,void *priv_data),
	void *priv_data){

	int n,i;
	pfring_zc_pkt_buff *pkt_buf;
	void *pkt_data;
	xm_if_pfring_t *pdrv = (xm_if_pfring_t*)pdrv_data->drv;
	xm_if_pfring_private_data_t *pfdrv_data = (xm_if_pfring_private_data_t*)pdrv_data;
	xm_if_pfring_context_t *pcontext = pdrv->pcontext;

	n = pfring_zc_recv_pkt_burst(pfdrv_data->rcv_zq,
		pfdrv_data->rcv_buf,
		pcontext->rx_per_n_buf,
		1);

	if(n>0){
	
		/* handle packets */
		for(i = 0; i<n;i++){
		
			pkt_buf = pfdrv_data->rcv_buf[i];
			pkt_data = pfring_zc_pkt_buff_data(pkt_buf,pfdrv_data->rcv_zq);

			packet_handle(pdrv_data,pkt_data,pkt_buf->len,(void*)pkt_buf,priv_data);

		}

		return n;

	}else{
	
		return 0;
	}
}

static int _pfring_send(xm_if_driver_private_data_t *pdrv_data,void *pkt_data,size_t pkt_len,size_t idx,
	void *priv_data xm_unused){

	int ret;
	pfring_zc_pkt_buff *pkt_buf;
	void *data;

	xm_if_pfring_t *pdrv = (xm_if_pfring_t*)pdrv_data->drv;
	xm_if_pfring_private_data_t *pfdrv_data = (xm_if_pfring_private_data_t*)pdrv_data; 
	xm_if_pfring_context_t *pcontext = pdrv->pcontext;

	idx = idx%pcontext->tx_per_n_buf;

	pkt_buf = pfdrv_data->send_bufs[idx];
	pkt_buf->len = pkt_len;

	data = pfring_zc_pkt_buff_data(pkt_buf,pfdrv_data->send_zq);

	memcpy(data,pkt_data,pkt_len);

	do{
	
		ret = pfring_zc_send_pkt(pfdrv_data->send_zq,&pkt_buf,0);
	}while(ret <0);

	return ret;
}

static xm_if_driver_private_data_t *  
_tx_pfring_private_data_create(xm_if_driver_t *drv,int socket xm_unused,void *priv_data xm_unused){

	xm_if_pfring_private_data_t *pfdrv_data;
	size_t i;
	
	xm_if_pfring_t *pdrv = (xm_if_pfring_t*)drv;
	xm_if_pfring_context_t *pcontext = pdrv->pcontext;

	/* Every thread has own memory pool */
	xm_pool_t *mp = xm_pool_create(4096);
	

	if(mp == NULL){
	
		xm_log(XM_LOG_ERR,"Create memory pool failed!");
		return NULL;
	}

	pfdrv_data = (xm_if_pfring_private_data_t*)xm_palloc(mp,sizeof(*pfdrv_data));
	pfdrv_data->drv_data.drv = drv;
	pfdrv_data->drv_data.mp = mp;
	pfdrv_data->drv_data.receive = NULL;
	pfdrv_data->drv_data.send = _pfring_send;
	pfdrv_data->drv_data.pkt_free = NULL;
	pfdrv_data->send_bufs = NULL;
	pfdrv_data->send_zq = NULL;
	pfdrv_data->rcv_zq = NULL;
	pfdrv_data->rcv_buf = NULL;


	pfdrv_data->send_bufs = (pfring_zc_pkt_buff**)xm_pcalloc(mp,
		sizeof(pfring_zc_pkt_buff*)*pcontext->tx_per_n_buf);

	for(i = 0; i<pcontext->tx_per_n_buf;i++){
	
		pfdrv_data->send_bufs[i] = pfring_zc_get_packet_handle(pdrv->zc);
		
		if(pfdrv_data->send_bufs[i] == NULL){
		
			xm_log(XM_LOG_ERR,"Could not get ZC packet handle!");
			xm_pool_destroy(mp);
			return NULL;
		}
	}

	pfdrv_data->send_zq = pfring_zc_create_queue(pdrv->zc,pcontext->tx_queue_len);
	if(pfdrv_data->send_zq == NULL){
	
		xm_log(XM_LOG_ERR,"Could not create send queue!");
		xm_pool_destroy(mp);
		return NULL;
	}
	
	xm_if_driver_private_data_add(drv,(xm_if_driver_private_data_t*)pfdrv_data);

	return (xm_if_driver_private_data_t*)pfdrv_data;
}

static xm_if_driver_private_data_t *  _rx_pfring_private_data_create(xm_if_driver_t *drv,int socket xm_unused,
	void *priv_data xm_unused){

	xm_if_pfring_private_data_t *pfdrv_data;
	size_t i;
	
	xm_if_pfring_t *pdrv = (xm_if_pfring_t*)drv;
	xm_if_pfring_context_t *pcontext = pdrv->pcontext;

	/* Every thread has own memory pool */
	xm_pool_t *mp = xm_pool_create(4096);
	

	if(mp == NULL){
	
		xm_log(XM_LOG_ERR,"Create memory pool failed!");
		return NULL;
	}

	pfdrv_data = (xm_if_pfring_private_data_t*)xm_palloc(mp,sizeof(*pfdrv_data));
	pfdrv_data->drv_data.drv = drv;
	pfdrv_data->drv_data.mp = mp;
	pfdrv_data->drv_data.receive = _pfring_rcv;
	pfdrv_data->drv_data.send = NULL;
	pfdrv_data->drv_data.pkt_free = NULL;
	pfdrv_data->send_bufs = NULL;
	pfdrv_data->send_zq = NULL;
	pfdrv_data->rcv_zq = NULL;
	pfdrv_data->rcv_buf = NULL;

	pfdrv_data->rcv_zq = pdrv->pf_rcv;
	pfdrv_data->rcv_buf = (pfring_zc_pkt_buff**)xm_pcalloc(mp,
		sizeof(pfring_zc_pkt_buff*)*pcontext->rx_per_n_buf);

	for(i = 0; i<pcontext->rx_per_n_buf;i++){
		
		pfdrv_data->rcv_buf[i] = pfring_zc_get_packet_handle(pdrv->zc);
		
		if(pfdrv_data->rcv_buf[i] == NULL){
			xm_log(XM_LOG_ERR,"Could not get packet handle!");
		
			xm_pool_destroy(mp);
			return NULL;
		}
	}

	xm_if_driver_private_data_add(drv,(xm_if_driver_private_data_t*)pfdrv_data);

	return (xm_if_driver_private_data_t*)pfdrv_data;
}

static void _pfring_fin(xm_if_driver_t *driver,void *priv_data xm_unused){

	xm_if_pfring_t *pdrv = (xm_if_pfring_t*)driver;

	xm_if_pfring_private_data_t *pdrv_data,**arr;

	size_t i;

	arr = (xm_if_pfring_private_data_t**)driver->drv_datas;
	
	for(i = 0; i<(size_t)driver->drv_datas->nelts;i++){
	
		pdrv_data = arr[i];

		xm_pool_destroy(pdrv_data->drv_data.mp);
	}


	pfring_zc_destroy_cluster(pdrv->zc);    
	
	 /* ok */
}

xm_if_driver_t * xm_if_pfring_create(xm_pool_t *mp,xm_if_t *interface){


	xm_if_pfring_t *pdriver = (xm_if_pfring_t *)xm_palloc(mp,sizeof(*pdriver));

	pdriver->driver.mp = mp;
	pdriver->driver.interface = interface;
	pdriver->driver.drv_datas = xm_array_make(mp,16,sizeof(xm_if_driver_private_data_t*));
	pdriver->driver.init = _pfring_init;
	pdriver->driver.start = _pfring_start;
	pdriver->driver.stop = _pfring_stop;
	pdriver->driver.fin = _pfring_fin;
	pdriver->driver.create_rx_private_data = _rx_pfring_private_data_create;
	pdriver->driver.create_tx_private_data = _tx_pfring_private_data_create;

	pdriver->mp = mp;
	pdriver->zc = NULL;
	pdriver->pf_send = NULL;
	pdriver->pf_rcv = NULL;
	pdriver->prefetches = NULL;

	/* create dpdk config context */
	pdriver->pcontext = xm_if_pfring_context_create(mp,interface->if_cfname); 

	if(pdriver->pcontext == NULL){
	

		xm_log(XM_LOG_ERR,"create pfring config context failed!");
		return NULL;
	}

	pdriver->total_buf_num = xm_if_pfring_total_buffers(pdriver);

	return (xm_if_driver_t*)pdriver;

}



