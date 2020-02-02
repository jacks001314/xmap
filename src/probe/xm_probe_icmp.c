/*
 * =====================================================================================
 *
 *       Filename:  xm_probe_icmp.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  11/19/2019 04:02:10 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jacksha (shajf), csp001314@163.com
 *   Organization:  
 *
 * =====================================================================================
 */
#include "xm_context.h"
#include "xm_packet.h"
#include <string.h>
#include "xm_probe.h"
#include "xm_validate.h"

static xm_context_t *gcontext;

static int icmp_probe_context_init(xm_context_t *context){

    gcontext = context;

    return 0;
}

static int icmp_probe_thread_init(void *pbuf,uint8_t *smac,uint8_t *dmac,uint16_t xm_unused dport,void xm_unused **arg_ptr){

    memset(pbuf, 0, MAX_PACKET_SIZE); 

	struct ether_header *eth_header = (struct ether_header *)pbuf;
	
    xm_make_eth_header(eth_header, smac, dmac);

	struct ip *ip_header = (struct ip *)(&eth_header[1]);
	
    uint16_t len = htons(sizeof(struct ip) + sizeof(struct icmp)-8);
	
    xm_make_ip_header(ip_header, IPPROTO_ICMP, len);

    struct icmp *icmp_header = (struct icmp *)(&ip_header[1]);

    xm_make_icmp_header(icmp_header);

    return EXIT_SUCCESS;
}

static int icmp_probe_make_packet(void *pbuf,size_t xm_unused *blen,uint32_t sip,uint32_t dip,uint8_t ttl,
        uint32_t *validation,int xm_unused probe_num, void xm_unused *arg){

	struct ether_header *eth_header = (struct ether_header *)pbuf;
	struct ip *ip_header = (struct ip *)(&eth_header[1]);
	struct icmp *icmp_header = (struct icmp *)(&ip_header[1]);

	uint16_t icmp_idnum = validation[1] & 0xFFFF;
	uint16_t icmp_seqnum = validation[2] & 0xFFFF;

	ip_header->ip_src.s_addr = sip;
	ip_header->ip_dst.s_addr = dip;
	ip_header->ip_ttl = ttl;

	icmp_header->icmp_id = icmp_idnum;
	icmp_header->icmp_seq = icmp_seqnum;

	icmp_header->icmp_cksum = 0;
	icmp_header->icmp_cksum = xm_icmp_checksum((unsigned short *)icmp_header);

	ip_header->ip_sum = 0;
	ip_header->ip_sum = xm_ip_checksum((unsigned short *)ip_header);

	return EXIT_SUCCESS;


}

static int icmp_probe_validate_packet(struct ip *ip_hdr,uint32_t len,uint32_t *sip,uint32_t *validation){

	if (ip_hdr->ip_p != IPPROTO_ICMP) {
		return 0;
	}

	// check if buffer is large enough to contain expected icmp header
	if (((uint32_t)4 * ip_hdr->ip_hl + ICMP_SMALLEST_SIZE) > len) {
		return 0;
	}

	struct icmp *icmp_h =
	    (struct icmp *)((char *)ip_hdr + 4 * ip_hdr->ip_hl);

	uint16_t icmp_idnum = icmp_h->icmp_id;
	uint16_t icmp_seqnum = icmp_h->icmp_seq;
	// ICMP validation is tricky: for some packet types, we must look inside
	// the payload
	if (icmp_h->icmp_type == ICMP_TIMXCEED ||
	    icmp_h->icmp_type == ICMP_UNREACH) {
		// Should have 16B TimeExceeded/Dest_Unreachable header +
		// original IP header + 1st 8B of original ICMP frame
		if ((4 * ip_hdr->ip_hl + ICMP_TIMXCEED_UNREACH_HEADER_SIZE +
		     sizeof(struct ip)) > len) {
			return 0;
		}
		struct ip *ip_inner = (struct ip *)((char *)icmp_h + 8);
		if (((uint32_t)4 * ip_hdr->ip_hl +
		     ICMP_TIMXCEED_UNREACH_HEADER_SIZE + 4 * ip_inner->ip_hl +
		     8 /*1st 8 bytes of original*/) > len) {
			return 0;
		}
		struct icmp *icmp_inner =
		    (struct icmp *)((char *)ip_inner + 4 * ip_hdr->ip_hl);
		// Regenerate validation and icmp id based off inner payload
		icmp_idnum = icmp_inner->icmp_id;
		icmp_seqnum = icmp_inner->icmp_seq;
        
        *sip = ip_hdr->ip_dst.s_addr;

		xm_validate_gen(ip_hdr->ip_dst.s_addr, ip_inner->ip_dst.s_addr,
			     (uint8_t *)validation);
	}

	// validate icmp id and seqnum
	if (icmp_idnum != (validation[1] & 0xFFFF)) {
		return 0;
	}

	if (icmp_seqnum != (validation[2] & 0xFFFF)) {
		return 0;
	}

	return 1;

}

static void icmp_probe_process_packet(void *pbuf,uint32_t xm_unused len,xm_store_t *xstore, uint32_t xm_unused *  validation){

	struct ip *ip_hdr = (struct ip *)(pbuf+sizeof(struct ether_header));
	struct icmp *icmp_hdr =
	    (struct icmp *)((char *)ip_hdr + 4 * ip_hdr->ip_hl);

    int success;
    const char *msg;

	switch (icmp_hdr->icmp_type) {
	
        case ICMP_ECHOREPLY:
            msg = "echoreply";
            success = 1;
		break;
        
        case ICMP_UNREACH:
            msg = "unreach";
            success = 0;
		break;
        
        case ICMP_SOURCEQUENCH:
            msg = "sourcequench";
            success = 0;
		break;

	    case ICMP_REDIRECT:
            msg = "redirect";
            success = 0;
		break;
        
        case ICMP_TIMXCEED:
            msg = "timxceed";
            success = 0;
		break;
        
        default:
            msg = "other";
            success = 0;
		break;
	}
    
    xm_store_map_start(xstore,"icmp",6);

    xm_store_write_uint8(xstore, "success", success);
    xm_store_write_kv(xstore,"msg",msg);
    xm_store_write_uint8(xstore, "type", icmp_hdr->icmp_type);
    xm_store_write_uint8(xstore, "code", icmp_hdr->icmp_code);

    xm_store_write_uint32(xstore, "id",ntohs(icmp_hdr->icmp_id));
	xm_store_write_uint32(xstore,"seq", ntohs(icmp_hdr->icmp_seq));

}

static void icmp_probe_dump_packet(FILE *fp, void *pbuf)
{
	struct ether_header *ethh = (struct ether_header *)pbuf;
	struct ip *iph = (struct ip *)&ethh[1];
	struct icmp *icmp_header = (struct icmp *)(&iph[1]);

	fprintf(fp,
		"icmp { type: %u | code: %u "
		"| checksum: %#04X | id: %u | seq: %u }\n",
		icmp_header->icmp_type, icmp_header->icmp_code,
		ntohs(icmp_header->icmp_cksum), ntohs(icmp_header->icmp_id),
		ntohs(icmp_header->icmp_seq));

	xm_dump_ip_header(fp, iph);
	xm_dump_eth_header(fp, ethh);
	fprintf(fp, "------------------------------------------------------\n");
}

xm_probe_t probe_icmp_echo = {
    .name = "icmp_echoscan",
    .packet_length = 62,
    .probe_type = PROBE_ICMP_ECHO,
    .port_args =0,
    .probe_context_init = icmp_probe_context_init,
    .probe_thread_init = icmp_probe_thread_init,
    .probe_make_packet = icmp_probe_make_packet,
    .probe_dump_packet = icmp_probe_dump_packet,
    .probe_validate_packet = icmp_probe_validate_packet,
    .probe_process_packet = icmp_probe_process_packet,
    .helptext = "Probe module that sends a icmp echo packet to a specific "
		"dstIP"};


