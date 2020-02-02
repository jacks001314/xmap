/*
 * =====================================================================================
 *
 *       Filename:  xm_context.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/30/2019 10:04:42 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jacksha (shajf), csp001314@163.com
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef XM_CONTEXT_H
#define XM_CONTEXT_H

typedef struct xm_context_t xm_context_t;

#include <stdlib.h>
#include <stdint.h>
#include <arpa/inet.h>
#include "xm_probe.h"
#include "xm_if.h"
#include "xm_mpool.h"
#include "xm_aesrand.h"
#include "xm_wblist.h"
#include "xopt.h"
#include "xm_headers.h"
#include "xm_pbm.h"
#include "xm_random.h"

#define MAX_PACKET_SIZE 4096
#define MAC_ADDR_LEN_BYTES 6


struct xm_context_t {

    const char *log_file;
	int log_level;

	uint16_t target_port;
	uint16_t source_port_first;
	uint16_t source_port_last;
	// maximum number of packets that the scanner will send before
	// terminating
	uint32_t max_targets;
	// maximum number of seconds that scanner will run before terminating
	uint32_t max_runtime;
	// maximum number of results before terminating
	uint32_t max_results;
	// name of network interface that
	// will be utilized for sending/receiving
	
    char *iface_send;
    char *iface_recv;
    char *iface_send_type;
    char *iface_recv_type;
    char *iface_send_cfname;
    char *iface_recv_cfname;

    // rate in packets per second
	// that the sender will maintain
	int rate;
	// rate in bits per second
	uint64_t bandwidth;
	// how many seconds after the termination of the sender will the
	// receiver continue to process responses
	int cooldown_secs;
	// number of sending threads
	uint8_t senders;
	// should use CLI provided randomization seed instead of generating
	// a random seed.
	int seed_provided;
	uint64_t seed;
	// generator of the cyclic multiplicative group that is utilized for
	// address generation
	uint32_t generator;
	// sharding options
	uint16_t shard_num;
	uint16_t total_shards;
	int packet_streams;
	
	char *probe_args;
	uint8_t probe_ttl;
	uint8_t gw_mac[MAC_ADDR_LEN_BYTES];
	uint8_t hw_mac[MAC_ADDR_LEN_BYTES];
	
	char **destination_cidrs;
	int destination_cidrs_len;
    
    uint32_t gw_ip;
	int gw_mac_set;
	int hw_mac_set;
	in_addr_t source_ip_addresses[256];
	uint32_t number_source_ips;
	uint32_t srcip_offset;

	int send_ip_pkts;
	char *blacklist_filename;
	char *whitelist_filename;
	char *list_of_ips_filename;
	uint32_t list_of_ips_count;
	int ignore_invalid_hosts;
	int filter_duplicates;
	int filter_unsuccessful;
	int num_retries;
	uint64_t total_allowed;
	uint64_t total_disallowed;
	int max_sendto_failures;
	float min_hitrate;
	int data_link_size;
    int recv_ready;

    char *core_mask;
    char *shm_fname;
    uint64_t shm_fsize;
    uint64_t shm_fentry_size;
    uint64_t shm_flush_timeout;

    struct {
        double start;
        double finish;
        uint32_t sent;
        uint32_t tried_sent;
        uint32_t blacklisted;
        uint32_t whitelisted;
        int warmup;
        int complete;
        uint32_t first_scanned;
        uint32_t max_targets;
        uint32_t sendto_failures;
        uint32_t max_index;
        uint8_t **list_of_ips_pbm;
    } xsend;

    struct {
        // valid responses classified as "success"
        uint32_t success_total;
        // unique IPs that sent valid responses classified as "success"
        uint32_t success_unique;
        // valid responses classified as "success"
        uint32_t app_success_total;
        // unique IPs that sent valid responses classified as "success"
        uint32_t app_success_unique;
        // valid responses classified as "success" received during cooldown
        uint32_t cooldown_total;
        // unique IPs that first sent valid "success"es during cooldown
        uint32_t cooldown_unique;
        // valid responses NOT classified as "success"
        uint32_t failure_total;
        // valid responses that passed the filter
        uint32_t filter_success;
        // how many packets did we receive that were marked as being the first
        // fragment in a stream
        uint32_t ip_fragments;
        // metrics about _only_ validate_packet
        uint32_t validation_passed;
        uint32_t validation_failed;

        int complete;  // has the scanner finished sending?
        double start;  // timestamp of when recv started
        double finish; // timestamp of when recv terminated

        // number of packets captured by pcap filter
        uint32_t pcap_recv;
        // number of packets dropped because there was no room in
        // the operating system's buffer when they arrived, because
        // packets weren't being read fast enough
        uint32_t pcap_drop;
        // number of packets dropped by the network interface or its driver.
        uint32_t pcap_ifdrop;
    } xrecv;
    
    pthread_mutex_t recv_ready_mutex;
    xm_pool_t *mp;
    xm_probe_t *probe; 

    xm_if_t *send_interface;
    xm_if_t *recv_interface;

    xm_wblist_t *wblist;
    xm_aesrand_t *aes;
};

extern xm_context_t * xm_context_create(xm_pool_t *mp,struct gengetopt_args_info *args);

extern void xm_parse_source_ip_addresses(xm_context_t *context,char given_string[]); 

extern uint32_t xm_parse_max_hosts(char *max_targets);

#endif /* XM_CONTEXT_H */
