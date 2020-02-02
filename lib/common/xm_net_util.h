/*
 *
 *      Filename: xm_net_util.h
 *
 *        Author: shajf,csp001314@gmail.com
 *   Description: ---
 *        Create: 2018-04-12 10:13:22
 * Last Modified: 2018-04-12 10:13:22
 */

#ifndef XM_NET_UTIL_H
#define XM_NET_UTIL_H

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <arpa/inet.h>

#define GW_BUFFER_SIZE 64000


static inline void xm_mac_addr_format(char *buffer,size_t bsize,uint8_t *addr_bytes){

	snprintf(buffer, bsize, "%02X:%02X:%02X:%02X:%02X:%02X",
		 addr_bytes[0],
		 addr_bytes[1],
		 addr_bytes[2],
		 addr_bytes[3],
		 addr_bytes[4],
		 addr_bytes[5]);

}

extern char* xm_ip_to_str(char *buffer,size_t bsize,uint32_t ip);


static inline void xm_ipv6_to_str(char *buffer,size_t bsize,unsigned char *addr){

	unsigned char ipv6[8];
    int i;

    for (i=0; i < 8; i++) 
        ipv6[i] = (addr[i*2] << 8) + addr[i*2+1];
	
	snprintf(buffer,bsize, "%x:%x:%x:%x:%x:%x:%x:%x", ipv6[0], ipv6[1], ipv6[2],
                                               ipv6[3], ipv6[4], ipv6[5],
                                               ipv6[6], ipv6[7]);
}

extern char * xm_get_default_iface(void);

extern int xm_get_iface_hw_addr(const char *iface, uint8_t *hw_mac);

extern int xm_get_iface_ip(const char *iface, struct in_addr *ip);

extern int xm_get_default_gw(struct in_addr *gw, char *iface);

extern int xm_get_hw_addr(struct in_addr *gw_ip, char *iface, unsigned char *hw_mac);

#endif /*XM_NET_UTIL_H*/
