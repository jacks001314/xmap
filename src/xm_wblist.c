/*
 * =====================================================================================
 *
 *       Filename:  xm_wblist.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/12/2019 11:52:07 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jacksha (shajf), csp001314@163.com
 *   Organization:  
 *
 * =====================================================================================
 */

#include "xm_wblist.h"
#include <errno.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "xm_log.h"

uint32_t xm_wblist_lookup_index(xm_wblist_t *wblist,uint64_t index)
{
	return ntohl(xm_iptree_lookup_index(wblist->wb_iptree, index, ADDR_ALLOWED));
}

// check whether a single IP address is allowed to be scanned.
//		1 => is allowed
//		0 => is not allowed
int xm_wblist_is_allowed(xm_wblist_t *wblist,uint32_t s_addr)
{
	return xm_iptree_lookup_ip(wblist->wb_iptree, ntohl(s_addr)) == ADDR_ALLOWED;
}

static void _add_iptree(xm_iptree_t *iptree,struct in_addr addr, int prefix_len, int value)
{
	xm_iptree_add(iptree, ntohl(addr.s_addr), prefix_len, value);
}

// e.g. wblist_add("128.255.134.0", 24)
void xm_blacklist_prefix(xm_wblist_t *wblist,char *ip, int prefix_len)
{
	struct in_addr addr;
	addr.s_addr = inet_addr(ip);
	_add_iptree(wblist->wb_iptree,addr, prefix_len, ADDR_DISALLOWED);
}

// whitelist a CIDR network allocation
void xm_whitelist_prefix(xm_wblist_t *wblist,char *ip, int prefix_len)
{
	struct in_addr addr;
	addr.s_addr = inet_addr(ip);
	_add_iptree(wblist->wb_iptree,addr, prefix_len, ADDR_ALLOWED);
}

static int init_from_string(xm_iptree_t *iptree,char *ip, int value)
{
	int prefix_len = 256;
	char *slash = strchr(ip, '/');
	if (slash) { // split apart network and prefix length
		*slash = '\0';
		char *end;
		char *len = slash + 1;
		errno = 0;
		prefix_len = strtol(len, &end, 10);
		if (end == len || errno != 0 || prefix_len < 0 ||
		    prefix_len > 256) {
			xm_log(XM_LOG_ERR,
				  "'%s' is not a valid prefix length", len);
			return -1;
		}
	}
	struct in_addr addr;
	int ret = -1;
	if (inet_aton(ip, &addr) == 0) {
		// Not an IP and not a CIDR block, try dns resolution
		struct addrinfo hint, *res;
		memset(&hint, 0, sizeof(hint));
		hint.ai_family = PF_INET;
		int r = getaddrinfo(ip, NULL, &hint, &res);
		if (r) {
			xm_log(XM_LOG_ERR,
				  "'%s' is not a valid IP "
				  "address or hostname",
				  ip);
			return -1;
		}
		
        // Got some addrinfo, let's see what happens
		struct addrinfo *aip;

        for (aip = res; aip; aip = aip->ai_next) {
			if (aip->ai_family != AF_INET) {
				continue;
			}
			struct sockaddr_in *sa =
			    (struct sockaddr_in *)aip->ai_addr;
			memcpy(&addr, &sa->sin_addr, sizeof(addr));
			xm_log(XM_LOG_DEBUG, "%s retrieved by hostname",
				  inet_ntoa(addr));
			ret = 0;
			_add_iptree(iptree,addr, prefix_len, value);
		}
	} else {
		_add_iptree(iptree,addr, prefix_len, value);
		return 0;
	}

	return ret;
}

static int init_from_file(xm_iptree_t *iptree,char *file, const char *name, int value,
			  int ignore_invalid_hosts)
{
	FILE *fp;
	char line[1000];

	fp = fopen(file, "r");
	if (fp == NULL) {
		xm_log(XM_LOG_ERR, "unable to open %s file: %s: %s", name, file,
			  strerror(errno));
        return -1;
	}

	while (fgets(line, sizeof(line), fp) != NULL) {
		char *comment = strchr(line, '#');
		if (comment) {
			*comment = '\0';
		}
		// hostnames can be up to 255 bytes
		char ip[256];
		if ((sscanf(line, "%256s", ip)) == EOF) {
			continue;
		}
		if (init_from_string(iptree,ip, value)) {
			if (!ignore_invalid_hosts) {
				xm_log(XM_LOG_ERR, "unable to parse %s file: %s",
					  name, file);
			}
		}
	}

	fclose(fp);

	return 0;
}

static void init_from_array(xm_iptree_t *iptree,char **cidrs, size_t len, int value,
			    int ignore_invalid_hosts)
{
    int i;

	for (i = 0; i < (int)len; i++) {
		int ret = init_from_string(iptree,cidrs[i], value);
		if (ret && !ignore_invalid_hosts) {
			xm_log(XM_LOG_ERR,"constraint",
				  "Unable to init from CIDR list");
		}
	}
}

uint64_t xm_wblist_count_allowed(xm_wblist_t *wblist)
{
	return xm_iptree_count_ips(wblist->wb_iptree, ADDR_ALLOWED);
}

uint64_t xm_wblist_count_not_allowed(xm_wblist_t *wblist)
{
	return xm_iptree_count_ips(wblist->wb_iptree, ADDR_DISALLOWED);
}

// network order
uint32_t xm_wblist_ip_to_index(xm_wblist_t *wblist,uint32_t ip)
{
	uint32_t ip_hostorder = ntohl(ip);
	return xm_iptree_lookup_ip(wblist->wb_iptree, ip_hostorder);
}

xm_wblist_t * xm_wblist_create(xm_pool_t *mp,char *whitelist_filename, char *blacklist_filename,
		   char **whitelist_entries, size_t whitelist_entries_len,
		   char **blacklist_entries, size_t blacklist_entries_len,
		   int ignore_invalid_hosts)
{

    xm_wblist_t *wblist = (xm_wblist_t*)xm_pcalloc(mp,sizeof(*wblist));
    wblist->mp = mp;

	if (whitelist_filename && whitelist_entries) {
		xm_log(XM_LOG_WARN,"whitelist",
			 "both a whitelist file and destination addresses "
			 "were specified. The union of these two sources "
			 "will be utilized.");
	}

	if (whitelist_filename || whitelist_entries_len > 0) {
		// using a whitelist, so default to allowing nothing
		wblist->wb_iptree = xm_iptree_create(ADDR_DISALLOWED);
        if(wblist->wb_iptree == NULL){
            xm_log(XM_LOG_ERR,"Cannot create iptree for wblist!");
            return NULL;
        }

		if (whitelist_filename) {
			init_from_file(wblist->wb_iptree,whitelist_filename, "whitelist",
				       ADDR_ALLOWED, ignore_invalid_hosts);
		}

		if (whitelist_entries) {
			init_from_array(wblist->wb_iptree,whitelist_entries,
					whitelist_entries_len, ADDR_ALLOWED,
					ignore_invalid_hosts);
		}
	} else {
		// no whitelist, so default to allowing everything
		wblist->wb_iptree = xm_iptree_create(ADDR_ALLOWED);
        if(wblist->wb_iptree == NULL){
            xm_log(XM_LOG_ERR,"Cannot create iptree for wblist!");
            return NULL;
        }
	}

	if (blacklist_filename) {
		init_from_file(wblist->wb_iptree,blacklist_filename, "blacklist", ADDR_DISALLOWED,
			       ignore_invalid_hosts);
	}

	if (blacklist_entries) {
		init_from_array(wblist->wb_iptree,blacklist_entries, blacklist_entries_len,
				ADDR_DISALLOWED, ignore_invalid_hosts);
	}

	init_from_string(wblist->wb_iptree,strdup("0.0.0.0"), ADDR_DISALLOWED);

	xm_iptree_paint_value(wblist->wb_iptree, ADDR_ALLOWED);

	uint64_t allowed = xm_wblist_count_allowed(wblist);

	xm_log(XM_LOG_DEBUG,
		  "%lu addresses (%0.0f%% of address "
		  "space) can be scanned",
		  allowed, allowed * 100. / ((long long int)1 << 32));

	if (!allowed) {
		xm_log(XM_LOG_ERR,"no addresses are eligible to be scanned in the "
			  "current configuration. This may be because the "
			  "wblist being used by xmap (%s) prevents "
			  "any addresses from receiving probe packets.",
			blacklist_filename);

		return NULL;
	}

	return wblist;
}


