/*
 * =====================================================================================
 *
 *       Filename:  xm_if_socket.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/31/2019 03:42:28 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jacksha (shajf), csp001314@163.com
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef XM_IF_SOCKET_H
#define XM_IF_SOCKET_H

typedef struct xm_if_socket_t xm_if_socket_t;
typedef struct xm_if_socket_private_data_t xm_if_socket_private_data_t;

#include <sys/ioctl.h>
#include <netpacket/packet.h>
#include "xm_mpool.h"
#include "xm_if.h"
#include "xm_headers.h"

struct xm_if_socket_t {

    xm_if_driver_t driver;
    xm_pool_t *mp;

};

struct xm_if_socket_private_data_t {

    xm_if_driver_private_data_t drv_data;

    int sock;
	struct sockaddr_ll sockaddr;
};

extern xm_if_driver_t *xm_if_socket_create(xm_pool_t *mp,xm_if_t *interface);

#endif /* XM_IF_SOCKET_H */
