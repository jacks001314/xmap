/*
 * =====================================================================================
 *
 *       Filename:  xm_if_pfring_context.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/26/2019 02:18:56 PM
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
#include "xm_config.h"

static void _if_pfring_context_init(xm_if_pfring_context_t *pcontext){

	pcontext->rx_n_queue = 0;
	pcontext->tx_n_queue = 0;
	pcontext->rx_per_n_buf = 128;
	pcontext->tx_per_n_buf = 128;
	pcontext->rx_buf_size = 1536;
	pcontext->tx_buf_size = 1536;
	pcontext->rx_queue_len = 8192;
	pcontext->tx_queue_len = 8192;
	pcontext->max_card_slots = 32768;
	pcontext->cluster_id = 9627;
	pcontext->mnt_dir = NULL;


}

static const char *cmd_rx_n_queue(cmd_parms *cmd xm_unused, void *_dcfg, const char *p1){


	char *endptr;

    xm_if_pfring_context_t *pcontext = (xm_if_pfring_context_t*)_dcfg;

    pcontext->rx_n_queue = (size_t)strtoul(p1,&endptr,10);
    
    return NULL;
}

static const char *cmd_tx_n_queue(cmd_parms *cmd xm_unused, void *_dcfg, const char *p1){


	char *endptr;

    xm_if_pfring_context_t *pcontext = (xm_if_pfring_context_t*)_dcfg;

    pcontext->tx_n_queue = (size_t)strtoul(p1,&endptr,10);
    
    return NULL;
}

static const char *cmd_rx_per_n_buf(cmd_parms *cmd xm_unused, void *_dcfg, const char *p1){


	char *endptr;

    xm_if_pfring_context_t *pcontext = (xm_if_pfring_context_t*)_dcfg;

    pcontext->rx_per_n_buf = (size_t)strtoul(p1,&endptr,10);
    
    return NULL;
}

static const char *cmd_tx_per_n_buf(cmd_parms *cmd xm_unused, void *_dcfg, const char *p1){


	char *endptr;

    xm_if_pfring_context_t *pcontext = (xm_if_pfring_context_t*)_dcfg;

    pcontext->tx_per_n_buf = (size_t)strtoul(p1,&endptr,10);
    
    return NULL;
}

static const char *cmd_rx_buf_size(cmd_parms *cmd xm_unused, void *_dcfg, const char *p1){


	char *endptr;

    xm_if_pfring_context_t *pcontext = (xm_if_pfring_context_t*)_dcfg;

    pcontext->rx_buf_size = (size_t)strtoul(p1,&endptr,10);
    
    return NULL;
}

static const char *cmd_tx_buf_size(cmd_parms *cmd xm_unused, void *_dcfg, const char *p1){


	char *endptr;

    xm_if_pfring_context_t *pcontext = (xm_if_pfring_context_t*)_dcfg;

    pcontext->tx_buf_size = (size_t)strtoul(p1,&endptr,10);
    
    return NULL;
}

static const char *cmd_rx_queue_len(cmd_parms *cmd xm_unused, void *_dcfg, const char *p1){


	char *endptr;

    xm_if_pfring_context_t *pcontext = (xm_if_pfring_context_t*)_dcfg;

    pcontext->rx_queue_len = (size_t)strtoul(p1,&endptr,10);
    
    return NULL;
}

static const char *cmd_tx_queue_len(cmd_parms *cmd xm_unused, void *_dcfg, const char *p1){


	char *endptr;

    xm_if_pfring_context_t *pcontext = (xm_if_pfring_context_t*)_dcfg;

    pcontext->tx_queue_len = (size_t)strtoul(p1,&endptr,10);
    
    return NULL;
}

static const char *cmd_max_card_slots(cmd_parms *cmd xm_unused, void *_dcfg, const char *p1){


	char *endptr;

    xm_if_pfring_context_t *pcontext = (xm_if_pfring_context_t*)_dcfg;

    pcontext->max_card_slots = (size_t)strtoul(p1,&endptr,10);
    
    return NULL;
}

static const char *cmd_cluster_id(cmd_parms *cmd xm_unused, void *_dcfg, const char *p1){


	char *endptr;

    xm_if_pfring_context_t *pcontext = (xm_if_pfring_context_t*)_dcfg;

    pcontext->cluster_id = (uint32_t)strtoul(p1,&endptr,10);
    
    return NULL;
}

static const char *cmd_mnt_dir(cmd_parms *cmd xm_unused, void *_dcfg, const char *p1){

    xm_if_pfring_context_t *pcontext = (xm_if_pfring_context_t*)_dcfg;

    pcontext->mnt_dir = p1;
    
    return NULL;
}

static const command_rec pfring_context_directives[] ={
    
    XM_INIT_TAKE1(
            "XMDPFRingRXNQueue",
            cmd_rx_n_queue,
            NULL,
            0,
            "set the number of receive queue"
            ),
	
    XM_INIT_TAKE1(
            "XMDPFRingTXNQueue",
            cmd_tx_n_queue,
            NULL,
            0,
            "set the number of transmit queue"
            ),

    XM_INIT_TAKE1(
            "XMDPFRingRXPerNBuf",
            cmd_rx_per_n_buf,
            NULL,
            0,
            "set the number of buffer per a receive queue"
            ),
    
	XM_INIT_TAKE1(
            "XMDPFRingTXPerNBuf",
            cmd_tx_per_n_buf,
            NULL,
            0,
            "set the number of buffer per a transmit queue"
            ),

    XM_INIT_TAKE1(
            "XMDPFRingRXBufSize",
            cmd_rx_buf_size,
            NULL,
            0,
            "set  one receive buffer size"
            ),
    
	XM_INIT_TAKE1(
            "XMDPFRingTXBufSize",
            cmd_tx_buf_size,
            NULL,
            0,
            "set  one transmit buffer size"
            ),
    
    XM_INIT_TAKE1(
            "XMDPFRingRXQueueLen",
            cmd_rx_queue_len,
            NULL,
            0,
            "set rx queue len"
            ),
    
	XM_INIT_TAKE1(
            "XMDPFRingTXQueueLen",
            cmd_tx_queue_len,
            NULL,
            0,
            "set tx queue len"
            ),

    XM_INIT_TAKE1(
            "XMDPFRingMaxCardSlots",
            cmd_max_card_slots,
            NULL,
            0,
            "set max card slots"
            ),

    XM_INIT_TAKE1(
            "XMDPFRingClusterID",
            cmd_cluster_id,
            NULL,
            0,
            "set the cluster id"
            ),

    XM_INIT_TAKE1(
            "XMDPFRingMntDir",
            cmd_mnt_dir,
            NULL,
            0,
            "set the hugepages mountpoint"
            ),

};

xm_if_pfring_context_t * xm_if_pfring_context_create(xm_pool_t *mp,const char *cfname){

	const char *msg;

	xm_if_pfring_context_t *pcontext = (xm_if_pfring_context_t*)xm_pcalloc(mp,sizeof(*pcontext));


	/* init default values */
	_if_pfring_context_init(pcontext);

    if(cfname == NULL||strlen(cfname) == 0){
        
		xm_log(XM_LOG_ERR,"Must specify the config path pfring config!");
		
		return NULL;
    }

    msg = xm_process_command_config(pfring_context_directives,(void*)pcontext,mp,mp,cfname);

    /*config failed*/
    if(msg!=NULL){

        xm_log(XM_LOG_ERR,"Config PF_Ring failed:%s",msg);
        return NULL;
    }

	return pcontext;

}

