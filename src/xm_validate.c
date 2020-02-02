/*
 * ZMap Copyright 2013 Regents of the University of Michigan
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 */

#include <stdint.h>
#include "rijndael-alg-fst.h"
#include "xm_random.h"
#include "xm_log.h"
#include "xm_validate.h"

#define AES_ROUNDS 10
#define AES_BLOCK_WORDS 4
#define AES_KEY_BYTES 16

static uint32_t aes_input[AES_BLOCK_WORDS];
static uint32_t aes_sched[(AES_ROUNDS + 1) * 4];

void xm_validate_init(void)
{
    int i;

	for (i = 0; i < AES_BLOCK_WORDS; i++) {
		aes_input[i] = 0;
	}

	uint8_t key[AES_KEY_BYTES];
	if (!xm_random_bytes(key, AES_KEY_BYTES)) {
		xm_log(XM_LOG_ERR,"validate couldn't get random bytes");
	}

	if (rijndaelKeySetupEnc(aes_sched, key, AES_KEY_BYTES * 8) !=
	    AES_ROUNDS) {
		xm_log(XM_LOG_ERR,"validate couldn't initialize AES key");
	}

}

void xm_validate_gen(uint32_t src, uint32_t dst,
		  uint8_t output[VALIDATE_BYTES])
{
	aes_input[0] = src;
	aes_input[1] = dst;
	rijndaelEncrypt(aes_sched, AES_ROUNDS, (uint8_t *)aes_input, output);
}
