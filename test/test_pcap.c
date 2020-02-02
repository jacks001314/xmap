/*
 *
 *      Filename: test_pcap.c
 *
 *        Author: shajf,csp001314@163.com
 *   Description: ---
 *        Create: 2017-03-01 09:42:49
 * Last Modified: 2017-03-01 09:46:26
 */


#include <stdio.h>
#include <stdlib.h>
#include "xm_core_pool.h"
#include "xm_log.h"
#include "xm_if_pcap.h"

static size_t pcount = 0;
static size_t pkt_bytes = 0;

static int _packet_handle(xm_if_driver_private_data_t *pdrv_data,void *pkt_data,size_t pkt_len,void *raw_pkt,void *priv_data){


	pcount +=1;
	pkt_bytes += pkt_len;
	xm_task_t *tsk = (xm_task_t*)priv_data;

	fprintf(stdout,"Task:%d,rcv total packets:%lu,bytes:%lu,pkt_len:%lu\n",
		(int)tsk->tsk_id,
		(unsigned long)pcount,
		(unsigned long)pkt_bytes,
		(unsigned long)pkt_len);


	return 0;
}

static int 
_task_run(xm_task_t *tsk,void *priv_data){

	xm_if_driver_private_data_t *pdr_data = (xm_if_driver_private_data_t*)priv_data;


	pdr_data->receive(pdr_data,_packet_handle,(void*)tsk);

	return TASK_RETURN_SLEEP;
}

static xm_task_t *
_make_task(xm_core_pool_t *cpool,uint32_t tsk_id,xm_if_driver_t *drv){


	xm_task_t *tsk = (xm_task_t*)xm_palloc(cpool->mp,sizeof(*tsk));
	tsk->priv_data = (void*)drv->create_rx_private_data(drv,0,NULL);

	if(tsk->priv_data == NULL){
	
		fprintf(stderr,"create pfring private data failed!\n");
		return NULL;
	}

	tsk->tsk_id = tsk_id;
	tsk->run = _task_run;
	tsk->init = NULL;
	tsk->exit = NULL;

	if(xm_core_pool_bind_task(cpool,tsk)){
		fprintf(stderr,"cannot bind task!\n");
		return NULL;
	}

	return tsk;
}

int main(int argc,char ** argv){


	xm_pool_t *mp;
	xm_core_pool_t *cpool;
	xm_if_driver_t *driver;
	xm_if_t port;

	uint32_t tsk_number; 
	const char *core_mask;
	uint32_t i;

	if(argc!=5){
	
		fprintf(stderr,"usage: main_name <task number> <core_mask> <port_name> <cfname>\n");
		return -1;
	}
	
	tsk_number = (uint32_t)atoi(argv[1]);
	core_mask = argv[2];
	port.if_name = argv[3];
	port.if_cfname = argv[4];

	mp = xm_pool_create(1024);

	xm_log_init(mp,"/tmp/test_fpring.log",8);
	
	cpool = xm_core_pool_create(mp,core_mask);
	if(cpool == NULL){
	
		fprintf(stderr,"create core pool failed!\n");
		return -1;
	}

	driver = xm_if_pcap_create(mp,&port);
	if(driver == NULL){
	
		fprintf(stderr,"create pcap driver failed!\n");
		return -1;
	}

	if(driver->init(driver,NULL)){
	
		fprintf(stderr,"init pfring driver failed!\n");
		return -1;
	}

	for(i = 0; i<tsk_number;i++){
	
		if(_make_task(cpool,i+1,driver) == NULL){
		
			break;
		}
	}


	if(xm_core_pool_slaves_setup(cpool)){
	
		fprintf(stderr,"setup all slave threads failed!\n");
		return -1;
	}

	xm_core_pool_wait_for_slaves(cpool);

	driver->fin(driver,NULL);

	return 0;
}

