/*
 */

#include "xm_random.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include "xm_log.h"

#define RANDSRC "/dev/urandom"

int xm_random_bytes(void *dst, size_t n)
{
	FILE *f = fopen(RANDSRC, "rb");
	if (!f) {
		xm_log(XM_LOG_ERR,"unable to read /dev/urandom: %s",
			  strerror(errno));
	}

	size_t r = fread(dst, n, 1, f);
	fclose(f);
	if (r < 1) {
		return 0;
	}

	return 1;
}
