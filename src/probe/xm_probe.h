/*
 * =====================================================================================
 *
 *       Filename:  xm_probe.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/30/2019 09:52:38 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jacksha (shajf), csp001314@163.com
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef XM_PROBE_H
#define XM_PROBE_H

typedef struct xm_probe_t xm_probe_t;

#define PACKET_VALID 1
#define PACKET_INVALID 0

#include "xm_store.h"
#include "xm_context.h"
#include "xm_packet.h"
#include "xm_net_util.h"

#define PROBE_TCP_SYN 1
#define PROBE_ICMP_ECHO 2
#define PROBE_ICMP_ECHO_TIME 3

struct xm_probe_t {

    const char *name;
    size_t packet_length;
    uint8_t probe_type;
    uint8_t port_args;

    int (*probe_context_init)(xm_context_t *context);
    int (*probe_thread_init)(void *pbuf,uint8_t *smac,uint8_t *dmac,uint16_t sport,void **arg_ptr);
    int (*probe_make_packet)(void *pbuf,size_t *blen,uint32_t sip,uint32_t dip,uint8_t ttl,uint32_t* validation,int probe_num,void *arg);
    void (*probe_dump_packet)(FILE *out,void *pbuf);

    int (*probe_validate_packet)(struct ip *ip_hdr,uint32_t len,uint32_t *sip,uint32_t *validation);

    void (*probe_process_packet)(void *pbuf,uint32_t len,xm_store_t *xstore,uint32_t *validation);

    const char *helptext;
};

extern xm_probe_t * xm_probe_get_by_name(const char *name);

extern void xm_probes_dump(void);

static inline void xm_probe_ip_dump(FILE *fp,struct ip *ip){

    char srcIP[32]={0};
    char dstIP[32]={0};

    xm_ip_to_str(srcIP,32,ip->ip_src.s_addr);
    xm_ip_to_str(dstIP,32,ip->ip_dst.s_addr);

    fprintf(fp,"%s,%s\n",srcIP,dstIP); 
}

static inline void xm_probe_ip_store(xm_probe_t *probe,xm_store_t *xstore,struct ip *ip){

    xm_store_map_start(xstore,NULL,3);
    
    xm_store_write_uint8(xstore,"type",probe->probe_type);

    xm_store_map_start(xstore,"ip",4);


	xm_store_write_uint32(xstore, "srcIP", (uint32_t)ip->ip_src.s_addr);
	xm_store_write_uint32(xstore, "dstIP", (uint32_t)ip->ip_dst.s_addr);
	xm_store_write_uint32(xstore, "ipid", ntohs(ip->ip_id));
	xm_store_write_uint32(xstore, "ttl", ip->ip_ttl);

}

#endif /* XM_PROBE_H */

