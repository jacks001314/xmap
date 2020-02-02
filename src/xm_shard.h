
#ifndef XM_SHARD_H
#define XM_SHARD_H

#include <stdint.h>

#include "xm_cyclic.h"
#include "xm_wblist.h"

#define XM_SHARD_DONE 0

typedef struct xm_shard_t xm_shard_t;

struct xm_shard_t {
	struct shard_state {
		uint32_t sent;
		uint32_t tried_sent;
		uint32_t blacklisted;
		uint32_t whitelisted;
		uint32_t failures;
		uint32_t first_scanned;
		uint32_t max_targets;
	} state;
	struct shard_params {
		uint64_t first;
		uint64_t last;
		uint64_t factor;
		uint64_t modulus;
	} params;
	
    uint64_t current;
    uint64_t max_index;

    xm_wblist_t *wblist;

};

extern xm_shard_t* xm_shard_create(xm_pool_t *mp, uint16_t shard_idx, uint16_t num_shards,
		uint8_t thread_idx, uint8_t num_threads,
		uint32_t max_total_targets, uint64_t max_index,xm_cycle_t *cycle,xm_wblist_t *wblist);

extern uint32_t xm_shard_get_cur_ip(xm_shard_t *shard);

extern uint32_t xm_shard_get_next_ip(xm_shard_t *shard);

#endif /* XM_SHARD_H */
