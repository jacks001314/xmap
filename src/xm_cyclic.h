
#ifndef XM_CYCLIC_H
#define XM_CYCLIC_H

#include <stdint.h>
#include <stddef.h>

#include "xm_aesrand.h"

typedef struct xm_cycle_t xm_cycle_t;
typedef struct xm_cyclic_group_t xm_cyclic_group_t;

// Represents a multiplicative cyclic group (Z/pZ)*
struct xm_cyclic_group_t {
	uint64_t prime;		    // p
	uint64_t known_primroot;    // Known primitive root of (Z/pZ)*
	size_t num_prime_factors;   // Length of num_prime_factors
	uint64_t prime_factors[10]; // Unique prime factors of (p-1)
} ;

// Represents a cycle in a group
struct xm_cycle_t {
	xm_cyclic_group_t *group;
	uint64_t generator;
	uint64_t order;
	uint32_t offset;
};

// Get a xm_cyclic_group_t of at least min_size.
extern xm_cyclic_group_t *xm_get_cyclic_group(uint64_t min_size);

// Generate cycle (find generator and inverse)
extern xm_cycle_t* xm_make_cycle(xm_pool_t *mp,xm_cyclic_group_t *group, xm_aesrand_t *aes);

// Perform the isomorphism from (Z/pZ)+ to (Z/pZ)*
// Given known primitive root of (Z/pZ)* n, with x in (Z/pZ)+, do:
//	f(x) = n^x mod p
//
// The isomorphism in the reverse direction is discrete log, and is
// therefore hard.
extern uint64_t xm_isomorphism(uint64_t additive_elt, xm_cyclic_group_t *mult_group);

#endif
