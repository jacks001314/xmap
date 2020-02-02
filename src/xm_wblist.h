/*
 * =====================================================================================
 *
 *       Filename:  xm_wblist.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/12/2019 11:24:25 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jacksha (shajf), csp001314@163.com
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef XM_WBLIST_H
#define XM_WBLIST_H

typedef struct xm_wblist_t xm_wblist_t;

#include "xm_iptree.h"
#include "xm_mpool.h"

struct xm_wblist_t {

    xm_pool_t *mp;

    xm_iptree_t *wb_iptree;

};

#define ADDR_DISALLOWED 0
#define ADDR_ALLOWED 1

extern uint32_t xm_wblist_lookup_index(xm_wblist_t *wblist,uint64_t index);

extern int xm_wblist_is_allowed(xm_wblist_t *wblist,uint32_t s_addr);

extern void xm_blacklist_prefix(xm_wblist_t *wblist,char *ip, int prefix_len);

extern void xm_whitelist_prefix(xm_wblist_t *wblist,char *ip, int prefix_len);

extern xm_wblist_t* xm_wblist_create(xm_pool_t *mp,char *whitelist, char *blacklist, char **whitelist_entries,
		   size_t whitelist_entries_len, char **wblist_entries,
		   size_t wblist_entries_len, int ignore_invalid_hosts);

extern uint64_t xm_wblist_count_allowed(xm_wblist_t *wblist);

extern uint64_t xm_wblist_count_not_allowed(xm_wblist_t *wblist);

extern uint32_t xm_wblist_ip_to_index(xm_wblist_t *wblist,uint32_t ip);


#endif /* XM_WBLIST_H */

