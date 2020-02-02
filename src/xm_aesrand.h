
#include <stdint.h>

#ifndef XM_AESRAND_H
#define XM_AESRAND_H

typedef struct xm_aesrand_t xm_aesrand_t;

#include "xm_mpool.h"
#define AES_ROUNDS 10
#define AES_BLOCK_WORDS 4
#define AES_KEY_BYTES 16
#define AES_KEY_BITS (AES_KEY_BYTES * 8)
#define OUTPUT_BYTES 16


struct xm_aesrand_t {

	uint32_t input[AES_BLOCK_WORDS];
	uint32_t sched[(AES_ROUNDS + 1) * 4];
	uint8_t output[OUTPUT_BYTES];

};

extern xm_aesrand_t *xm_aesrand_create_from_random(xm_pool_t *mp);

extern xm_aesrand_t *xm_aesrand_create_from_seed(xm_pool_t *mp,uint64_t seed);

extern uint64_t xm_aesrand_getword(xm_aesrand_t *aes);


#endif
