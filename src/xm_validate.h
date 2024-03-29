/*
 * ZMap Copyright 2013 Regents of the University of Michigan
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef XM_VALIDATE_H
#define XM_VALIDATE_H

#define VALIDATE_BYTES 16

#include <stdint.h>

extern void xm_validate_init(void);

extern void xm_validate_gen(uint32_t src, uint32_t dst,
		  uint8_t output[VALIDATE_BYTES]);

#endif //XM_VALIDATE_H
