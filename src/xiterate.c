/*
 * ZMap Copyright 2016 Regents of the University of Michigan
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 */

/*
 * xiterate is a simple utility that will iteratate over the IPv4
 * space in a pseudo-random fashion, utilizing the sharding capabilities * of
 * ZMap.
 */

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "xm_wblist.h"
#include "xm_log.h"
#include "xm_random.h"
#include "xm_shard.h"
#include "xm_validate.h"
#include "xitopt.h"

typedef struct xit_conf xit_conf;

struct xit_conf {
	char *blacklist_filename;
	char *whitelist_filename;
	char **destination_cidrs;
	int destination_cidrs_len;
	char *log_filename;
	int check_duplicates;
	int ignore_errors;
	int verbosity;

	// sharding options
	uint16_t shard_num;
	uint16_t total_shards;
	uint64_t seed;
	xm_pool_t *mp;
	xm_aesrand_t *aes;
	xm_wblist_t *wblist;
	uint32_t max_hosts;
};

#define SET_BOOL(DST, ARG)                                                     \
	{                                                                      \
		if (args.ARG##_given) {                                        \
			(DST) = 1;                                             \
		};                                                             \
	}


static xm_shard_t* _create_shard(xit_conf *conf){
	
	uint64_t num_addrs = xm_wblist_count_allowed(conf->wblist);

	xm_cyclic_group_t *group = xm_get_cyclic_group(num_addrs);
	xm_cycle_t *cycle = xm_make_cycle(conf->mp,group,conf->aes);

	
	return xm_shard_create(conf->mp,conf->shard_num, conf->total_shards,0,
			   1,num_addrs,num_addrs,cycle,conf->wblist);
	
}

static double  _parse_max_hosts(char *max_targets)
 {
     char *end;
     double v = strtod(max_targets, &end);
     if (end == max_targets ) {
         printf("argparse can't convert max-targets to a number\n");
         return 0;
     }
 
     if (end[0] == '%' && end[1] == '\0') {
         // treat as percentage
         v = v * ((unsigned long long int)1 << 32) / 100.;
     } else if (end[0] != '\0') {
         printf("eargparse extra characters after max-targets\n");
         return 0;
     }
 
     if (v <= 0) {
         return 0;
     } else if (v >= ((unsigned long long int)1 << 32)) {
         return 0xFFFFFFFF;
     } else {
         return v;
     }
 }

int main(int argc, char **argv)
{
	xit_conf conf;
    xm_pool_t *mp;

	memset(&conf, 0, sizeof(struct xit_conf));
	conf.verbosity = 3;
	conf.ignore_errors = 0;

	struct gengetopt_args_info args;
	struct cmdline_parser_params *params;
	params = cmdline_parser_params_create();
	params->initialize = 1;
	params->override = 0;
	params->check_required = 0;

	if (cmdline_parser_ext(argc, argv, &args, params) != 0) {
		exit(0);
	}

	// Handle help text and version
	if (args.help_given) {
		cmdline_parser_print_help();
		exit(0);
	}
	if (args.version_given) {
		cmdline_parser_print_version();
		exit(0);
	}

	// Set the log file and metadata file
	if (args.log_file_given) {
		conf.log_filename = strdup(args.log_file_arg);
    }else{
        conf.log_filename = "/tmp/xiterate.log";
    }
	if (args.verbosity_given) {
		conf.verbosity = args.verbosity_arg;
    }else{
        conf.verbosity = XM_LOG_DEBUG;
    }

	// Read the boolean flags
	SET_BOOL(conf.ignore_errors, ignore_blacklist_errors);

    mp = xm_pool_create(1024);
    conf.mp = mp;
    xm_log_init(mp,conf.log_filename,conf.verbosity);

	// Blacklist and whitelist
	if (args.blacklist_file_given) {
		conf.blacklist_filename = strdup(args.blacklist_file_arg);
	}
	if (args.whitelist_file_given) {
		conf.whitelist_filename = strdup(args.whitelist_file_arg);
	}
	conf.destination_cidrs = args.inputs;
	conf.destination_cidrs_len = args.inputs_num;
	// max targets
	if (args.max_targets_given) {
		conf.max_hosts = _parse_max_hosts(args.max_targets_arg);
	}

	// sanity check blacklist file
	if (conf.blacklist_filename) {
		printf("Xiterate blacklist file at %s to be used\n",
			  conf.blacklist_filename);
	} else {
		printf("Xiterate no blacklist file specified\n");
	}
	if (conf.blacklist_filename &&
	    access(conf.blacklist_filename, R_OK) == -1) {
		printf("Xiterate unable to read specified blacklist file (%s)\n",
			  conf.blacklist_filename);
		return -1;
	}

	// sanity check whitelist file
	if (conf.whitelist_filename) {
		printf("Xiterate whitelist file at %s to be used\n",
			  conf.whitelist_filename);
	} else {
		printf("Xiterate no whitelist file specified\n");
	}

	if (conf.whitelist_filename &&
	    access(conf.whitelist_filename, R_OK) == -1) {
		printf("Xiterate unable to read specified whitelist file (%s)\n",
			  conf.whitelist_filename);
		return -1;
	}

	// parse blacklist and whitelist
    xm_wblist_t *wblist = xm_wblist_create(mp,conf.whitelist_filename,
	conf.blacklist_filename, conf.destination_cidrs, conf.destination_cidrs_len,
	NULL,0,conf.ignore_errors);

	if (wblist == NULL) {
		printf("Xiterate unable to initialize blacklist / whitelist\n");
		return -1;
	}
    conf.wblist = wblist;

	// Set up sharding
	conf.shard_num = 0;
	conf.total_shards = 1;
	if ((args.shard_given || args.shards_given) && !args.seed_given) {
		printf("Xiterate Need to specify seed if sharding a scan\n");
		return -1;
	}
	if (args.shard_given ^ args.shards_given) {
		printf(
		    "Xiterate Need to specify both shard number and total number of shards\n");
		return -1;
	}

	if (args.shard_given) {
		conf.shard_num = args.shard_arg;
	}

	if (args.shards_given) {
		conf.total_shards = args.shards_arg;
	}
	if (conf.shard_num >= conf.total_shards) {
		printf("xiterate With %u total shards, shard number (%u)\n"
			  " must be in range [0, %u)\n",
			  conf.total_shards, conf.shard_num, conf.total_shards);
		return -1;
	}

	printf("xiterate Initializing sharding (%d shards, shard number %d, seed %llu)\n",
	    conf.total_shards, conf.shard_num, conf.seed);

	// Check for a random seed
	if (args.seed_given) {
		conf.seed = args.seed_arg;
	} else {
		if (!xm_random_bytes(&conf.seed, sizeof(uint64_t))) {
			printf("xiterate unable to generate random bytes "
					      "needed for seed\n");
			return -1;
		}
	}

	conf.aes = xm_aesrand_create_from_seed(mp,conf.seed);

	xm_shard_t *shard = _create_shard(&conf);

	uint32_t next_int = xm_shard_get_cur_ip(shard);
	struct in_addr next_ip;

	uint32_t count;

	for (count = 0; next_int; ++count) {
		if (conf.max_hosts && count >= conf.max_hosts) {
			break;
		}

		next_ip.s_addr = next_int;
		printf("%s\n", inet_ntoa(next_ip));
		next_int = xm_shard_get_next_ip(shard);
	}
	return 0;
}
