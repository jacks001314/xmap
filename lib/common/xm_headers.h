/*
 * =====================================================================================
 *
 *       Filename:  xm_headers.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/31/2019 04:05:03 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jacksha (shajf), csp001314@163.com
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef XM_HEADERS_H
#define XM_HEADERS_H

#ifdef __APPLE__
#pragma GCC diagnostic ignored "-Wflexible-array-extensions"
#include <dnet.h>
#pragma GCC diagnostic warning "-Wflexible-array-extensions"
#endif

#ifndef __FAVOR_BSD
#define __FAVOR_BSD 2
#endif
#ifndef __USE_BSD
#define __USE_BSD
#endif

#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>

#if defined(__NetBSD__)
#define ICMP_UNREACH_PRECEDENCE_CUTOFF ICMP_UNREACH_PREC_CUTOFF
#include <net/if_ether.h>
#else
#include <net/ethernet.h>
#endif

#include <netdb.h>
#include <net/if.h>
#include <ifaddrs.h> // NOTE: net/if.h MUST be included BEFORE ifaddrs.h
#include <arpa/inet.h>

#define MAC_ADDR_LEN ETHER_ADDR_LEN

#endif /* XM_HEADERS_H */

