
#ifndef XM_PACKET_H
#define XM_PACKET_H

#include <stdio.h>
#include <stdint.h>
#include "xm_headers.h"

#define MAX_PACKET_SIZE 4096
#define ICMP_SMALLEST_SIZE 5
#define ICMP_TIMXCEED_UNREACH_HEADER_SIZE 8

extern void xm_make_eth_header(struct ether_header *ethh, uint8_t *src, uint8_t *dst);

extern void xm_make_ip_header(struct ip *iph, uint8_t protocol, uint16_t len);

extern void xm_make_tcp_header(struct tcphdr *tcp_header, uint16_t dest_port, uint16_t th_flags);

extern void xm_make_icmp_header(struct icmp *buf);

extern void xm_make_udp_header(struct udphdr *udp_header, uint16_t dest_port,
		     uint16_t len);

extern void xm_dump_ip_header(FILE *fp, struct ip *iph);

extern void xm_dump_eth_header(FILE *fp, struct ether_header *ethh);

static inline uint16_t xm_in_checksum(uint16_t *ip_pkt, int len)
{
	unsigned long sum = 0;
    int nwords;

	for (nwords = len / 2; nwords > 0; nwords--) {
		sum += *ip_pkt++;
	}
	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	return (uint16_t)(~sum);
}

static inline uint16_t xm_ip_checksum(uint16_t *buf)
{
	return xm_in_checksum(buf, (int)sizeof(struct ip));
}

static inline uint16_t xm_icmp_checksum(uint16_t *buf)
{
	return xm_in_checksum(buf, (int)sizeof(struct icmp));
}

static inline  uint16_t xm_tcp_checksum(uint16_t len_tcp,
						     uint32_t saddr,
						     uint32_t daddr,
						     struct tcphdr *tcp_pkt)
{
	uint16_t *src_addr = (uint16_t *)&saddr;
	uint16_t *dest_addr = (uint16_t *)&daddr;

	unsigned char prot_tcp = 6;
	unsigned long sum = 0;
	int nleft = len_tcp;
	uint16_t *w;

	w = (uint16_t *)tcp_pkt;
	// calculate the checksum for the tcp header and tcp data
	while (nleft > 1) {
		sum += *w++;
		nleft -= 2;
	}
	// if nleft is 1 there ist still on byte left.
	// We add a padding byte (0xFF) to build a 16bit word
	if (nleft > 0) {
		sum += *w & ntohs(0xFF00);
	}
	// add the pseudo header
	sum += src_addr[0];
	sum += src_addr[1];
	sum += dest_addr[0];
	sum += dest_addr[1];
	sum += htons(len_tcp);
	sum += htons(prot_tcp);
	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	// Take the one's complement of sum
	return (uint16_t)(~sum);
}

// Returns 0 if dst_port is outside the expected valid range, non-zero otherwise
static  inline int
xm_check_dst_port(uint16_t port, int num_ports,uint16_t source_port_first,uint16_t source_port_last,uint32_t packet_streams, uint32_t *validation)
{
	if (port > source_port_last || port < source_port_first) {
		return 0;
	}
	int32_t to_validate = port - source_port_first;
	int32_t min = validation[1] % num_ports;
	int32_t max = (validation[1] + packet_streams - 1) % num_ports;

	return (((max - min) % num_ports) >= ((to_validate - min) % num_ports));
}

static inline uint16_t
xm_get_src_port(int num_ports, int probe_num,uint16_t source_port_first, uint32_t *validation)
{
	return source_port_first +
	       ((validation[1] + probe_num) % num_ports);
}

// Note: caller must free return value
extern char *xm_make_ip_str(uint32_t ip);

#endif
