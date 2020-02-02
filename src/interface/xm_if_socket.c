/*
 * =====================================================================================
 *
 *       Filename:  xm_if_socket.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/31/2019 04:12:36 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jacksha (shajf), csp001314@163.com
 *   Organization:  
 *
 * =====================================================================================
 */
#include "xm_if_socket.h"
#include "xm_log.h"

static int _socket_init(xm_if_driver_t *drv xm_unused,void *priv_data xm_unused){

    return 0;
}

static int _socket_start(xm_if_driver_t *drv xm_unused,void *priv_data xm_unused){

	return 0;
}

static int _socket_stop(xm_if_driver_t *drv xm_unused,void *priv_data xm_unused){

	return 0;
}

static void _socket_fin(xm_if_driver_t *drv,void *priv_data xm_unused){

	size_t i;
	xm_if_socket_private_data_t *pdrv_data ,**arr;

	arr = (xm_if_socket_private_data_t**)drv->drv_datas;

	for(i = 0; i<(size_t)drv->drv_datas->nelts;i++){
	
		pdrv_data = arr[i];
		xm_pool_destroy(pdrv_data->drv_data.mp);
	}

}

static int _socket_send(xm_if_driver_private_data_t *pdrv_data,void *pkt_data,size_t pkt_len,size_t idx xm_unused,
        void *priv_data xm_unused){

	xm_if_socket_private_data_t *sdata = (xm_if_socket_private_data_t*)pdrv_data;
	struct sockaddr_ll *sockaddr = &sdata->sockaddr;

	return sendto(sdata->sock,pkt_data,pkt_len, 0, (struct sockaddr *)sockaddr,
		      sizeof(struct sockaddr_ll));	
}

static int _private_data_socket_init(xm_if_socket_private_data_t *psocket,int send_ip_pkts,const char *if_name,const char *if_mac){
	
	struct ifreq if_idx;
	int sock;
	struct sockaddr_ll *sockaddr = &psocket->sockaddr;

	if (send_ip_pkts) {
		sock = socket(AF_PACKET, SOCK_DGRAM, htons(ETH_P_ALL));
	} else {
		sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	}

	if (sock <= 0) {
		xm_log(XM_LOG_ERR,"send couldn't create socket. "
			  "Are you root? Error: %s",
			  strerror(errno));
		return -1;
	}
	
	psocket->sock = sock;
	
	// get source interface index
	memset(&if_idx, 0, sizeof(struct ifreq));
	if (strlen(if_name) >= IFNAMSIZ) {
		xm_log(XM_LOG_ERR,"send device interface name (%s) too long",
			  if_name);
		return -1;
	}

	strncpy(if_idx.ifr_name, if_name, IFNAMSIZ - 1);
	if (ioctl(sock, SIOCGIFINDEX, &if_idx) < 0) {
		return -1;
	}

	int ifindex = if_idx.ifr_ifindex;

	// destination address for the socket
	memset((void *)sockaddr, 0, sizeof(struct sockaddr_ll));
	sockaddr->sll_ifindex = ifindex;
	sockaddr->sll_halen = ETH_ALEN;

	if (send_ip_pkts) {
		sockaddr->sll_protocol = htons(ETHERTYPE_IP);
	}

	memcpy(sockaddr->sll_addr, if_mac, ETH_ALEN);
	return 0;	
}

static xm_if_driver_private_data_t * _socket_private_data_create(xm_if_driver_t *drv,int send_ip_pkts,
	void *priv_data xm_unused){

	xm_if_socket_private_data_t *pdrv_data;
	xm_pool_t *mp;
	xm_if_t *interface = drv->interface;

	mp = xm_pool_create(4096);

	if(mp == NULL){
	
		xm_log(XM_LOG_ERR,"Create memory pool failed for socket private data!");
		return NULL;
	}

	pdrv_data = (xm_if_socket_private_data_t*)xm_pcalloc(mp,sizeof(*pdrv_data));
	pdrv_data->drv_data.mp = mp;
	pdrv_data->drv_data.drv = drv;
	pdrv_data->drv_data.receive = NULL;
	pdrv_data->drv_data.pkt_free = NULL;
	pdrv_data->drv_data.send = _socket_send;

	if(_private_data_socket_init(pdrv_data,send_ip_pkts,interface->if_name,(const char*)interface->if_mac)){
		xm_log(XM_LOG_ERR,"create socket failed!");
		xm_pool_destroy(mp);
		return NULL;
	}

	xm_if_driver_private_data_add(drv,(xm_if_driver_private_data_t*)pdrv_data);

	return (xm_if_driver_private_data_t*)pdrv_data;
}

xm_if_driver_t *xm_if_socket_create(xm_pool_t *mp,xm_if_t *interface){


    xm_if_socket_t *sdriver = (xm_if_socket_t*)xm_palloc(mp,sizeof(*sdriver));

	sdriver->driver.mp = mp;
	sdriver->driver.interface = interface;
	sdriver->driver.drv_datas = xm_array_make(mp,16,sizeof(xm_if_driver_private_data_t*));
	sdriver->driver.init = _socket_init;
	sdriver->driver.start = _socket_start;
	sdriver->driver.stop = _socket_stop;
	sdriver->driver.fin = _socket_fin;
	sdriver->driver.create_rx_private_data = NULL;
	sdriver->driver.create_tx_private_data = _socket_private_data_create;

	sdriver->mp = mp;

    return (xm_if_driver_t*)sdriver;
}

