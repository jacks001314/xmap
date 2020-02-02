/*
 * =====================================================================================
 *
 *       Filename:  xm_pbm.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/29/2019 01:26:09 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jacksha (shajf), csp001314@163.com
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef XM_PBM_H
#define XM_PBM_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

extern uint8_t **xm_pbm_init(void);

extern int xm_pbm_check(uint8_t **b, uint32_t v);

extern void xm_pbm_set(uint8_t **b, uint32_t v);

extern uint32_t xm_pbm_load_from_file(uint8_t **b, char *file);

#endif /* XM_PBM_H */

