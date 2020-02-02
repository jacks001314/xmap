/*
 * =====================================================================================
 *
 *       Filename:  xm_probe_tcpsynscan.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/30/2019 10:55:22 AM
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

static xm_context_t *gcontext;
static uint32_t num_ports;

static int synscan_probe_context_init(xm_context_t *context){
	
    gcontext = context;
    num_ports = context->source_port_last - context->source_port_first + 1;
	return 0;
}

static int synscan_probe_thread_init(void *pbuf,uint8_t *smac,uint8_t *dmac,uint16_t dport,void xm_unused **arg_ptr){

	memset(pbuf, 0, MAX_PACKET_SIZE);

	struct ether_header *eth_header = (struct ether_header *)pbuf;
	
    xm_make_eth_header(eth_header, smac, dmac);

	struct ip *ip_header = (struct ip *)(&eth_header[1]);
	
    uint16_t len = htons(sizeof(struct ip) + sizeof(struct tcphdr));
	
    xm_make_ip_header(ip_header, IPPROTO_TCP, len);
	
    struct tcphdr *tcp_header = (struct tcphdr *)(&ip_header[1]);
	xm_make_tcp_header(tcp_header, dport, TH_SYN);

	return 0;

}

static int synscan_probe_make_packet(void *pbuf,size_t xm_unused *blen,uint32_t sip,uint32_t dip,uint8_t ttl,uint32_t *validation,int probe_num,void xm_unused *arg){

	struct ether_header *eth_header = (struct ether_header *)pbuf;
	struct ip *ip_header = (struct ip *)(&eth_header[1]);
	struct tcphdr *tcp_header = (struct tcphdr *)(&ip_header[1]);
	uint32_t tcp_seq = validation[0];

	ip_header->ip_src.s_addr = sip;
	ip_header->ip_dst.s_addr = dip;
	ip_header->ip_ttl = ttl;

	tcp_header->th_sport =
	    htons(xm_get_src_port(num_ports, probe_num, gcontext->source_port_first,validation));

	tcp_header->th_seq = tcp_seq;
	tcp_header->th_sum = 0;
	tcp_header->th_sum =xm_tcp_checksum(sizeof(struct tcphdr), ip_header->ip_src.s_addr,
			 ip_header->ip_dst.s_addr, tcp_header);

	ip_header->ip_sum = 0;
	ip_header->ip_sum = xm_ip_checksum((uint16_t *)ip_header);

	return 0;

}

static int synscan_probe_validate_packet(struct ip *ip_hdr,uint32_t len,uint32_t xm_unused *sip,uint32_t *validation){

	if (ip_hdr->ip_p != IPPROTO_TCP) {
		return 0;
	}

	if ((4 * ip_hdr->ip_hl + sizeof(struct tcphdr)) > len) {
		// buffer not large enough to contain expected tcp header
		return 0;
	}

	struct tcphdr *tcp =
	    (struct tcphdr *)((char *)ip_hdr + 4 * ip_hdr->ip_hl);

	uint16_t sport = tcp->th_sport;
	uint16_t dport = tcp->th_dport;
	
    // validate source port
	if (ntohs(sport) != gcontext->target_port) {
		return 0;
	}
	// validate destination port
	if (!xm_check_dst_port(ntohs(dport), num_ports, gcontext->source_port_first,
                gcontext->source_port_last,gcontext->packet_streams,validation)) {
		return 0;
	}

	// We treat RST packets different from non RST packets
	if (tcp->th_flags & TH_RST) {
		// For RST packets, recv(ack) == sent(seq) + 0 or + 1
		if (htonl(tcp->th_ack) != htonl(validation[0]) &&
		    htonl(tcp->th_ack) != htonl(validation[0]) + 1) {
			return 0;
		}
	} else {
		// For non RST packets, recv(ack) == sent(seq) + 1
		if (htonl(tcp->th_ack) != htonl(validation[0]) + 1) {
			return 0;
		}
	}

	return 1;
}

static void synscan_probe_process_packet(void *pbuf,uint32_t xm_unused len,xm_store_t *xstore,uint32_t xm_unused *validation){

	struct ip *ip_hdr = (struct ip *)(pbuf+sizeof(struct ether_header));
	struct tcphdr *tcp =
	    (struct tcphdr *)((char *)ip_hdr + 4 * ip_hdr->ip_hl);


    int success;
    const char *msg;

	if (tcp->th_flags & TH_RST) { // RST packet
	
        success = 0;
        msg = "rst";

    } else { // SYNACK packet
	
        success = 1;
        msg = "synacn";
    }
	
    xm_store_map_start(xstore,"tcp",7);
    xm_store_write_uint8(xstore, "success", success);
	xm_store_write_kv(xstore,"msg",msg);

    xm_store_write_uint16(xstore, "sport", (uint16_t)ntohs(tcp->th_sport));
	xm_store_write_uint16(xstore, "dport", (uint16_t)ntohs(tcp->th_dport));
    xm_store_write_uint32(xstore, "seqnum", (uint32_t)ntohl(tcp->th_seq));
	xm_store_write_uint32(xstore, "acknum", (uint32_t)ntohl(tcp->th_ack));
	xm_store_write_uint32(xstore, "window", (uint32_t)ntohs(tcp->th_win));

}

static void synscan_probe_dump_packet(FILE *fp,void *pbuf){

	struct ether_header *ethh = (struct ether_header *)pbuf;
	struct ip *iph = (struct ip *)&ethh[1];
	struct tcphdr *tcph = (struct tcphdr *)&iph[1];

	fprintf(fp,
		"tcp { source: %u | dest: %u | seq: %u | checksum: %#04X }\n",
		ntohs(tcph->th_sport), ntohs(tcph->th_dport),
		ntohl(tcph->th_seq), ntohs(tcph->th_sum));
	
    xm_dump_ip_header(fp, iph);
	xm_dump_eth_header(fp, ethh);
	
    fprintf(fp, "------------------------------------------------------\n");

}

xm_probe_t probe_tcp_synscan = {
    .name = "tcp_synscan",
    .packet_length = 54,
    .probe_type = PROBE_TCP_SYN,
    .port_args =1,
    .probe_context_init = synscan_probe_context_init,
    .probe_thread_init = synscan_probe_thread_init,
    .probe_make_packet = synscan_probe_make_packet,
    .probe_dump_packet = synscan_probe_dump_packet,
    .probe_validate_packet = synscan_probe_validate_packet,
    .probe_process_packet = synscan_probe_process_packet,
    .helptext = "Probe module that sends a TCP SYN packet to a specific "
		"port. Possible classifications are: synack and rst. A "
		"SYN-ACK packet is considered a success and a reset packet "
		"is considered a failed response."};


