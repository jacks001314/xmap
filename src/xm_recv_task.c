/*
 * =====================================================================================
 *
 *       Filename:  xm_recv_task.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/08/2019 03:41:12 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jacksha (shajf), csp001314@163.com
 *   Organization:  
 *
 * =====================================================================================
 */
#include "xm_recv_task.h"
#include "xm_log.h"
#include "xm_validate.h"
#include "xm_pbm.h"
#include "xm_util.h"
#include "xm_net_util.h"

static uint8_t fake_eth_hdr[65535];

static int _recv_task_init(xm_task_t *task xm_unused,void *priv_data xm_unused){

	return 0;
}

static int _recv_task_exit(xm_task_t *task xm_unused,void *priv_data xm_unused){

	return 0;
}

static void _dump_ip(uint32_t srcIP,uint32_t dstIP,const char *s){

    if((srcIP == 83994816&&dstIP==369207488)||(srcIP==369207488&&dstIP==83994816))
        return;

    if(srcIP!=369207488&&dstIP!=369207488)
        return;

    char sb[64]={0};
    char db[64]={0};
    xm_ip_to_str(sb,64,srcIP);
    xm_ip_to_str(db,64,dstIP);

    printf("%s:%lu,%lu,%s,%s\n", s,(unsigned long)srcIP,(unsigned long)dstIP,sb,db); 
}

static int _recv_packet_handle(xm_if_driver_private_data_t *pdrv_data xm_unused,void *pkt_data,size_t pkt_len,void *pkt_buf xm_unused,void *priv_data){
	
	xm_recv_task_t *recv_task = (xm_recv_task_t*)priv_data;
	xm_context_t *context = recv_task->context;

	if ((sizeof(struct ip) + context->data_link_size) > pkt_len) {
		// buffer not large enough to contain ethernet
		// and ip headers. further action would overrun buf
		return 0;
	}

	struct ip *ip_hdr = (struct ip *)(pkt_data+context->data_link_size);

	uint32_t src_ip = ip_hdr->ip_src.s_addr;

	uint32_t validation[VALIDATE_BYTES / sizeof(uint8_t)];


   // _dump_ip(ip_hdr->ip_dst.s_addr,ip_hdr->ip_src.s_addr,"before");

	// TODO: for TTL exceeded messages, ip_hdr->saddr is going to be
	// different and we must calculate off potential payload message instead
	xm_validate_gen(ip_hdr->ip_dst.s_addr, ip_hdr->ip_src.s_addr,
		     (uint8_t *)validation);

	if (!context->probe->probe_validate_packet(
		ip_hdr,
		pkt_len - (context->send_ip_pkts?0:sizeof(struct ether_header)),
		&src_ip, validation)) {
		context->xrecv.validation_failed++;
		return 0;

	} else {
		context->xrecv.validation_passed++;
	}
    
    //_dump_ip(ip_hdr->ip_dst.s_addr,ip_hdr->ip_src.s_addr,"after");

	// woo! We've validated that the packet is a response to our scan
	int is_repeat = xm_pbm_check(recv_task->seen, ntohl(src_ip));

	// track whether this is the first packet in an IP fragment.
	if (ip_hdr->ip_off & IP_MF) {
		context->xrecv.ip_fragments++;
	}
    
    context->xrecv.success_total++;

    if (!is_repeat) {
        context->xrecv.success_unique++;
        xm_pbm_set(recv_task->seen, ntohl(src_ip));
    }

    if (context->xsend.complete) {
        context->xrecv.cooldown_total++;
        if (!is_repeat) {
            context->xrecv.cooldown_unique++;
        }
    }

    if(is_repeat&&context->filter_duplicates){

        return 0;
    }
	
	context->xrecv.filter_success++;
    // HACK:
	// probe modules expect the full ethernet frame
	// in process_packet. For VPN, we only get back an IP frame.
	// Here, we fake an ethernet frame (which is initialized to
	// have ETH_P_IP proto and 00s for dest/src).
	if (context->send_ip_pkts) {
		if (pkt_len > sizeof(fake_eth_hdr)) {
			pkt_len = sizeof(fake_eth_hdr);
		}

		memcpy(&fake_eth_hdr[sizeof(struct ether_header)],
		       pkt_data + context->data_link_size, pkt_len);
		pkt_data = (void*)fake_eth_hdr;
	}

	xm_probe_ip_store(context->probe,recv_task->xstore,ip_hdr);
	
    context->probe->probe_process_packet(pkt_data,pkt_len,recv_task->xstore, validation);

    xm_store_flush(recv_task->xstore,context->probe->probe_type);

    return 0;
}

static int _recv_task_run(xm_task_t *task,void *priv_data xm_unused){

    xm_recv_task_t *recv_task = (xm_recv_task_t*)task;
	xm_context_t *context = recv_task->context;
    xm_if_driver_private_data_t *if_driver = recv_task->if_driver;

	pthread_mutex_lock(&context->recv_ready_mutex);
	context->recv_ready = 1;
	pthread_mutex_unlock(&context->recv_ready_mutex);

	context->xrecv.start = now();

	if (context->max_results == 0) {
		context->max_results = -1;
	}
	
    do {
        
        if_driver->receive(if_driver,_recv_packet_handle,(void*)recv_task);

        if (context->max_results &&
			    context->xrecv.filter_success >= context->max_results) {
				break;
        }

	} while (
	    !(context->xsend.complete && (now() - context->xsend.finish > context->cooldown_secs)));

	context->xrecv.finish = now();

	context->xrecv.complete = 1;

	xm_log(XM_LOG_DEBUG,"recv thread finished");

	return TASK_RETURN_EXIT;

}

xm_recv_task_t * xm_recv_task_create(xm_context_t *context,int thread_id){


    xm_recv_task_t *recv_task = (xm_recv_task_t*)xm_pcalloc(context->mp,sizeof(*recv_task));

	recv_task->task.tsk_id = (uint32_t)thread_id;
	recv_task->task.run = _recv_task_run;
	recv_task->task.init = _recv_task_init;
	recv_task->task.exit = _recv_task_exit;

    recv_task->thread_id = thread_id;
    recv_task->context = context;
    context->recv_interface->if_driver->init(context->recv_interface->if_driver,NULL);
    recv_task->if_driver = context->recv_interface->if_driver->create_rx_private_data(context->recv_interface->if_driver,0,NULL); 

	if(recv_task->if_driver == NULL){
		xm_log(XM_LOG_ERR,"Cannot create recv interface failed!");
		return NULL;
	}

    recv_task->seen = xm_pbm_init();

    recv_task->xstore = xm_store_create(context->mp,context->shm_fname,context->shm_fsize,context->shm_fentry_size,
            context->shm_flush_timeout);

    if(recv_task->xstore == NULL){

        xm_log(XM_LOG_ERR,"Cannot create recv xstore failed!");
        return NULL;
    }

    return recv_task;
}
