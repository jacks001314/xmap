/*
 * =====================================================================================
 *
 *       Filename:  xm_if_pcap.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/26/2019 03:28:10 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jacksha (shajf), csp001314@163.com
 *   Organization:  
 *
 * =====================================================================================
 */
#include "xm_if_pcap.h"

#include "xm_log.h"

static int _pcap_init(xm_if_driver_t *drv,void *priv_data xm_unused){

	char errBuf[PCAP_ERRBUF_SIZE];
	xm_if_pcap_t *pdrv = (xm_if_pcap_t*)drv;
	xm_if_t *interface = drv->interface;
	xm_if_pcap_context_t *pcontext = pdrv->pcontext;
	struct bpf_program filter;  

	pdrv->pcap = pcap_open_live(interface->if_name,
			pcontext->snaplen,
			pcontext->promisc_mode, 
			pcontext->wait_ms, 
			errBuf);

	if(pdrv->pcap == NULL){
	
		xm_log(XM_LOG_ERR,"Cannot open interface:%s to capture packets!",
			interface->if_name);
		return -1;
	}

	/*  construct a filter */  
	if(pcontext->filter_text){
	
		if(pcap_compile(pdrv->pcap, &filter, pcontext->filter_text, 1, 0)){
		

			xm_log(XM_LOG_ERR,"Compile the pcap filter text:[%s] failed!",
				pcontext->filter_text);
			return -1;
		}

		pcap_setfilter(pdrv->pcap, &filter);	
	}  

	/* ok */
	return 0;
}

static int _pcap_start(xm_if_driver_t *drv xm_unused,void *priv_data xm_unused){

	return 0;
}

static int _pcap_stop(xm_if_driver_t *drv xm_unused,void *priv_data xm_unused){


	return 0;
}

static size_t _pcap_rcv(xm_if_driver_private_data_t *drv_data,
	int (*packet_handle)(xm_if_driver_private_data_t *pdrv_data,void *pkt_data,size_t pkt_len,void *pkt_buf,void *priv_data),
	void *priv_data){

  struct pcap_pkthdr packet;  
	xm_if_pcap_t *pdrv = (xm_if_pcap_t*)drv_data->drv;
	/* wait a packet to arrive */  

	void *pkt_data =NULL;

	do{ 
		pkt_data = (void*)pcap_next(pdrv->pcap, &packet);
		
	}while(pkt_data == NULL);

	packet_handle(drv_data,pkt_data,packet.caplen,pkt_data,priv_data);

	return 1;
}

static void _pcap_fin(xm_if_driver_t *drv,void *priv_data xm_unused){

	size_t i;
	xm_if_pcap_private_data_t *pdrv_data ,**arr;

	arr = (xm_if_pcap_private_data_t**)drv->drv_datas;

	for(i = 0; i<(size_t)drv->drv_datas->nelts;i++){
	
		pdrv_data = arr[i];
		xm_pool_destroy(pdrv_data->drv_data.mp);
	}

}

static xm_if_driver_private_data_t * _pcap_private_data_create(xm_if_driver_t *drv,int socket xm_unused,
	void *priv_data xm_unused){


	xm_if_pcap_private_data_t *pdrv_data;
	xm_pool_t *mp;

	mp = xm_pool_create(4096);

	if(mp == NULL){
	
		xm_log(XM_LOG_ERR,"Create memory pool failed for pcap private data!");
		return NULL;
	}

	pdrv_data = (xm_if_pcap_private_data_t*)xm_pcalloc(mp,sizeof(*pdrv_data));
	pdrv_data->drv_data.mp = mp;
	pdrv_data->drv_data.drv = drv;
	pdrv_data->drv_data.receive = _pcap_rcv;
	pdrv_data->drv_data.pkt_free = NULL;
	pdrv_data->drv_data.send = NULL;


	xm_if_driver_private_data_add(drv,(xm_if_driver_private_data_t*)pdrv_data);

	return (xm_if_driver_private_data_t*)pdrv_data;
}

xm_if_driver_t * xm_if_pcap_create(xm_pool_t *mp,xm_if_t *interface){


	xm_if_pcap_t *pdrv = (xm_if_pcap_t*)xm_pcalloc(mp,sizeof(*pdrv));

	pdrv->driver.mp = mp;
	pdrv->driver.drv_datas = xm_array_make(mp,16,sizeof(xm_if_pcap_private_data_t*));
	pdrv->driver.interface = interface;
	pdrv->driver.init = _pcap_init;
	pdrv->driver.start = _pcap_start;
	pdrv->driver.create_rx_private_data = _pcap_private_data_create;
	pdrv->driver.create_tx_private_data = NULL;
	pdrv->driver.stop = _pcap_stop;
	pdrv->driver.fin = _pcap_fin;
	pdrv->mp = mp;

	pdrv->pcap = NULL;

	pdrv->pcontext = xm_if_pcap_context_create(mp,interface->if_cfname);
	if(pdrv->pcontext == NULL){
	
		xm_log(XM_LOG_ERR,"Create pcap config context failed!");
		return NULL;
	}
	


	return (xm_if_driver_t*)pdrv;
}


