/*
 * =====================================================================================
 *
 *       Filename:  xm_iptree.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/12/2019 09:40:46 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jacksha (shajf), csp001314@163.com
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef XM_IPTREE_H
#define XM_IPTREE_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct xm_iptree_t xm_iptree_t;
typedef struct xm_node_t xm_node_t;

struct xm_node_t {

    xm_node_t *l;
    xm_node_t *r;
    uint32_t value;
    uint64_t count;
};

#define RADIX_LENGTH 20

struct xm_iptree_t {

	xm_node_t *root;     
	
    uint32_t *radix;  
	
    size_t radix_len; 
	
    int painted;      
	
    uint32_t paint_value; 
};

extern xm_iptree_t * xm_iptree_create(uint32_t value);

extern void xm_iptree_destroy(xm_iptree_t *iptree);

extern void xm_iptree_add(xm_iptree_t *iptree, uint32_t prefix, int len, uint32_t value);

extern uint32_t xm_iptree_lookup_ip(xm_iptree_t *iptree, uint32_t address);

extern uint64_t xm_iptree_count_ips(xm_iptree_t *iptree, uint32_t value);

extern uint32_t xm_iptree_lookup_index(xm_iptree_t *iptree, uint64_t index,
				 uint32_t value);

extern void xm_iptree_paint_value(xm_iptree_t *iptree, uint32_t value);

#endif /* XM_IPTREE_H */
