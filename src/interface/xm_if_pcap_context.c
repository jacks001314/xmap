/*
 * =====================================================================================
 *
 *       Filename:  xm_if_pcap_context.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/26/2019 03:19:58 PM
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
#include "xm_config.h"

static void _pcap_context_init(xm_if_pcap_context_t *pcontext){

	pcontext->snaplen = 65535;
	pcontext->wait_ms = 0;
	pcontext->promisc_mode = 1;
	pcontext->filter_text = NULL;
}

static const char * cmd_snaplen(cmd_parms *cmd xm_unused,void *_dcfg,const char *p1){

	char *endptr;
	xm_if_pcap_context_t *pcontext = (xm_if_pcap_context_t*)_dcfg;

	pcontext->snaplen = (size_t)strtoul(p1,&endptr,10);

	return NULL;
}

static const char * cmd_wait_ms(cmd_parms *cmd xm_unused,void *_dcfg,const char *p1){

	char *endptr;
	xm_if_pcap_context_t *pcontext = (xm_if_pcap_context_t*)_dcfg;

	pcontext->wait_ms = (uint64_t)strtoul(p1,&endptr,10);

	return NULL;
}

static const char * cmd_promisc_mode(cmd_parms *cmd xm_unused,void *_dcfg,const char *p1){

	xm_if_pcap_context_t *pcontext = (xm_if_pcap_context_t*)_dcfg;

	if(strcasecmp(p1,"on") == 0){
	
		pcontext->promisc_mode = 1;
	}else{
	
		pcontext->promisc_mode = 0;
	}

	return NULL;
}

static const char * cmd_filter_text(cmd_parms *cmd xm_unused,void *_dcfg,const char *p1){

	xm_if_pcap_context_t *pcontext = (xm_if_pcap_context_t*)_dcfg;

	pcontext->filter_text = p1;

	return NULL;
}

static const command_rec pcap_context_directives[] ={
    
    XM_INIT_TAKE1(
            "XMPCAPSnapLen",
            cmd_snaplen,
            NULL,
            0,
            "set libpcap snaplen value"
            ),
    
	XM_INIT_TAKE1(
            "XMPCAPWaitMS",
            cmd_wait_ms,
            NULL,
            0,
            "set libpcap capture packets timeout by ms!"
            ),

    XM_INIT_TAKE1(
            "XMPCAPPromiscMode",
            cmd_promisc_mode,
            NULL,
            0,
            "set libpcap promisc mode on/off"
            ),

    XM_INIT_TAKE1(
            "XMPCAPFilterText",
            cmd_filter_text,
            NULL,
            0,
            "set libpcap filter text"
            ),

};

xm_if_pcap_context_t * xm_if_pcap_context_create(xm_pool_t *mp,const char *cfname){


	const char *msg;

	xm_if_pcap_context_t *pcontext = (xm_if_pcap_context_t*)xm_pcalloc(mp,sizeof(*pcontext));


	/* init default values */
	_pcap_context_init(pcontext);

    if(cfname == NULL||strlen(cfname) == 0){
        
		xm_log(XM_LOG_ERR,"Must specify the config path pcap config!");
		
		return NULL;
    }

    msg = xm_process_command_config(pcap_context_directives,(void*)pcontext,mp,mp,cfname);

    /*config failed*/
    if(msg!=NULL){

        xm_log(XM_LOG_ERR,"Config PCAP config file failed:%s",msg);
        return NULL;
    }


	return pcontext;
}




