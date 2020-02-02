/*
 * =====================================================================================
 *
 *       Filename:  xm_pbm.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/29/2019 01:32:12 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jacksha (shajf), csp001314@163.com
 *   Organization:  
 *
 * =====================================================================================
 */

#include <errno.h>
#include "xm_pbm.h"
#include "xm_log.h"
#include "xm_xalloc.h"

#define NUM_VALUES 0xFFFFFFFF
#define PAGE_SIZE_IN_BITS 0x10000
#define PAGE_SIZE_IN_BYTES (PAGE_SIZE_IN_BITS / 8)
#define NUM_PAGES 0x10000
#define PAGE_MASK 0xFFFF

uint8_t **xm_pbm_init(void)
{
	uint8_t **retv = xcalloc(NUM_PAGES, sizeof(void *));
	return retv;
}

static inline int bm_check(uint8_t *bm, uint16_t v)
{
	uint16_t page_idx = (v >> 3);
	uint8_t bit_idx = (uint8_t)(v & 0x07);
	return bm[page_idx] & (1 << bit_idx);
}

static inline void bm_set(uint8_t *bm, uint16_t v)
{
	uint16_t page_idx = (v >> 3);
	uint8_t bit_idx = (uint8_t)(v & 0x07);
	bm[page_idx] |= (1 << bit_idx);
}

int xm_pbm_check(uint8_t **b, uint32_t v)
{
	uint32_t top = v >> 16;
	uint32_t bottom = v & PAGE_MASK;
	return b[top] && bm_check(b[top], bottom);
}

void xm_pbm_set(uint8_t **b, uint32_t v)
{
	uint16_t top = (uint16_t)(v >> 16);
	uint16_t bottom = (uint16_t)(v & PAGE_MASK);
	if (!b[top]) {
		uint8_t *bm = xmalloc(PAGE_SIZE_IN_BYTES);
		memset(bm, 0, PAGE_SIZE_IN_BYTES);
		b[top] = bm;
	}
	bm_set(b[top], bottom);
}

uint32_t xm_pbm_load_from_file(uint8_t **b, char *file)
{
	if (!b) {
		xm_log(XM_LOG_ERR, "load_from_file called with NULL PBM");
	}
	if (!file) {
		xm_log(XM_LOG_ERR, "load_from_file called with NULL filename");
	}
	FILE *fp = fopen(file, "r");
	if (fp == NULL) {
		xm_log(XM_LOG_ERR, "unable to open file: %s: %s", file,
			  strerror(errno));
	}
	char line[1000];
	uint32_t count = 0;
	while (fgets(line, sizeof(line), fp)) {
		char *comment = strchr(line, '#');
		if (comment) {
			*comment = '\0';
		}
		struct in_addr addr;
		if (inet_aton(line, &addr) != 1) {
			xm_log(XM_LOG_ERR,"unable to parse IP address: %s",
				  line);
		}

		xm_pbm_set(b, addr.s_addr);
		++count;
	}
	fclose(fp);
	return count;
}

