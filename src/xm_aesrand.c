#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

#include "rijndael-alg-fst.h"
#include "xm_random.h"
#include "xm_log.h"
#include "xm_aesrand.h"

static xm_aesrand_t *_aesrand_init(xm_pool_t *mp,uint8_t *key)
{
	xm_aesrand_t *aes = xm_palloc(mp,sizeof(xm_aesrand_t));
	memset(&aes->input, 0, AES_BLOCK_WORDS * 4);
	if (rijndaelKeySetupEnc(aes->sched, key, AES_KEY_BITS) != AES_ROUNDS) {
		xm_log(XM_LOG_ERR, "could not initialize AES key");
        return NULL;
	}

	memset(aes->output, 0, OUTPUT_BYTES);
	return aes;
}

xm_aesrand_t *xm_aesrand_create_from_seed(xm_pool_t *mp,uint64_t seed)
{
	
    uint8_t i;
    uint8_t key[AES_KEY_BYTES];
	memset(key, 0, AES_KEY_BYTES);
	for (i = 0; i < sizeof(seed); ++i) {
		key[i] = (uint8_t)((seed >> 8 * i) & 0xFF);
	}

	return _aesrand_init(mp,key);
}

xm_aesrand_t *xm_aesrand_create_from_random(xm_pool_t *mp)
{
	uint8_t key[AES_KEY_BYTES];
	if (!xm_random_bytes(key, AES_KEY_BYTES)) {
		xm_log(XM_LOG_ERR, "Couldn't get random bytes");
        return NULL;
	}

	return _aesrand_init(mp,key);
}

uint64_t xm_aesrand_getword(xm_aesrand_t *aes)
{
	memcpy(aes->input, aes->output, sizeof(aes->input));
	rijndaelEncrypt(aes->sched, AES_ROUNDS, (uint8_t *)aes->input,
			aes->output);
	uint64_t retval;
	memcpy(&retval, aes->output, sizeof(retval));
	return retval;
}

