/*
 * =====================================================================================
 *
 *       Filename:  xm_if.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/01/2019 10:42:16 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jacksha (shajf), csp001314@163.com
 *   Organization:  
 *
 * =====================================================================================
 */
#include "xm_if.h"
#include "xm_net_util.h"
#include "xm_log.h"
#include "xm_if_pfring.h"
#include "xm_if_socket.h"
#include "xm_if_pcap.h"

static inline int _get_if_type(const char *name){

    if(!strcasecmp(name,IF_DPDK_NAME)){
        return IF_DPDK;
    }
    else if(!strcasecmp(name,IF_PFRING_NAME)){

        return IF_PFRING;
    } else if(!strcasecmp(name,IF_LIBPCAP_NAME)){

        return IF_LIBPCAP;
    }else if(!strcasecmp(name,IF_LINUX_NAME)){

        return IF_LINUX;
    }else{
        xm_log(XM_LOG_ERR,"Unknown if.type:%s",name);
        return -1;
    }

}

static int _make_driver(xm_if_t *interface){

	xm_if_driver_t *if_drv = NULL;

	switch(interface->if_type){

		case IF_PFRING:
			if_drv = xm_if_pfring_create(interface->mp,interface);
			break;

		case IF_LIBPCAP:
			if_drv = xm_if_pcap_create(interface->mp,interface); 
			break;

		case IF_LINUX:
			if_drv = xm_if_socket_create(interface->mp,interface); 
			break;

		default:
			xm_log(XM_LOG_ERR,"Unknown interface driver!");
			return -1; 
	}

	if(if_drv == NULL){
		xm_log(XM_LOG_ERR,"Cannot create interface driver!");
		return -1;
	}

	interface->if_driver = if_drv;

	return 0;
}

xm_if_t *xm_if_create(xm_pool_t *mp,const char *if_name,const char *if_type,const char *if_cfname){


    if(if_name == NULL||strlen(if_name)==0){
        xm_log(XM_LOG_ERR,"Must Specify network interface!");
        return NULL;
    }

    xm_if_t *interface = (xm_if_t*)xm_palloc(mp,sizeof(*interface));

    interface->if_type = _get_if_type(if_type);
    if(interface->if_type == -1){

        return NULL;
    }


    interface->mp = mp;
    interface->if_name = if_name;
    interface->if_cfname = if_cfname;

	if (xm_get_iface_hw_addr(if_name, interface->if_mac)) {
			xm_log(XM_LOG_ERR,"send could not retrieve hardware address for "
				  "interface: %s",if_name);
		return NULL;
	}

	if (xm_get_iface_ip(if_name, &interface->if_addr) < 0) {
		xm_log(XM_LOG_ERR,"could not detect default IP address for %s."
				  " Try specifying a source address (-S).",
				  if_name);
		return NULL;
	}

	if(-1 == _make_driver(interface))
		return NULL;

    return interface;
}

