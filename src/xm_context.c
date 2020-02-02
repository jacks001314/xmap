/*
 * =====================================================================================
 *
 *       Filename:  xm_context.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/30/2019 02:30:08 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jacksha (shajf), csp001314@163.com
 *   Organization:  
 *
 * =====================================================================================
 */
#include "xm_context.h"

#include <stdio.h>
#include <arpa/inet.h>
#include "xm_log.h"
#include "xm_util.h"
#include "xm_net_util.h"
#include "xm_validate.h"

static in_addr_t string_to_ip_address(char *t)
{
	in_addr_t r = inet_addr(t);
	if (r == INADDR_NONE) {
		xm_log(XM_LOG_ERR,"send invalid ip address: `%s'", t);
	}

	return r;
}

static void add_to_array(xm_context_t *context,char *to_add)
{
	if (context->number_source_ips >= 256) {
		xm_log(XM_LOG_ERR,"parse over 256 source IP addresses provided");
        return;
    }

	xm_log(XM_LOG_DEBUG,"SEND ipaddress: %s", to_add);

	context->source_ip_addresses[context->number_source_ips] =
	    string_to_ip_address(to_add);

	context->number_source_ips++;
}

void xm_parse_source_ip_addresses(xm_context_t *context,char given_string[])
{
	char *dash = strchr(given_string, '-');
	char *comma = strchr(given_string, ',');
	if (dash && comma) {
		*comma = '\0';
		xm_parse_source_ip_addresses(context,given_string);
		xm_parse_source_ip_addresses(context,comma + 1);
	} else if (comma) {
		while (comma) {
			*comma = '\0';
			add_to_array(context,given_string);
			given_string = comma + 1;
			comma = strchr(given_string, ',');
			if (!comma) {
				add_to_array(context,given_string);
			}
		}
	} else if (dash) {
		*dash = '\0';
		xm_log(XM_LOG_DEBUG,"SEND address: %s\n", given_string);
		xm_log(XM_LOG_DEBUG,"SEND address: %s\n", dash + 1);
		in_addr_t start = ntohl(string_to_ip_address(given_string));
		in_addr_t end = ntohl(string_to_ip_address(dash + 1)) + 1;
		while (start != end) {
			struct in_addr temp;
			temp.s_addr = htonl(start);
			add_to_array(context,strdup(inet_ntoa(temp)));
			start++;
		}
	} else {
		add_to_array(context,given_string);
	}
}

uint32_t xm_parse_max_hosts(char *max_targets)
{
	char *end;
	double v = strtod(max_targets, &end);
	if (end == max_targets ) {
		xm_log(XM_LOG_ERR,"argparse can't convert max-targets to a number");
		return 0;
	}

	if (end[0] == '%' && end[1] == '\0') {
		// treat as percentage
		v = v * ((unsigned long long int)1 << 32) / 100.;
	} else if (end[0] != '\0') {
		xm_log(XM_LOG_ERR,"eargparse extra characters after max-targets");
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

static void _context_init(xm_context_t *context) {

    context->log_level = XM_LOG_INFO;
    context->source_port_first = 32768; // (these are the default
    context->source_port_last = 61000; //	 ephemeral range on Linux)
    context->blacklist_filename = NULL;
    context->whitelist_filename = NULL;
    context->list_of_ips_filename = NULL;
    context->list_of_ips_count = 0;
    context->target_port = 0;
    context->max_targets = 0xFFFFFFFF;
    context->max_runtime = 0;
    context->max_results = 0;
    context->iface_send = NULL;
    context->iface_recv = NULL;
    context->iface_send_type = NULL;
    context->iface_recv_type = NULL;
    context->iface_send_cfname = NULL;
    context->iface_recv_cfname = NULL;
    context->rate = -1;
    context->bandwidth = 0;
    context->cooldown_secs = 0;
    context->senders = 1;
    context->packet_streams = 1;
    context->seed_provided = 0;
    context->seed = 0;
    context->probe_args = NULL;
    context->probe_ttl = MAXTTL;
    context->gw_ip = 0;
    context->gw_mac_set = 0;
    context->hw_mac_set = 0;
    context->number_source_ips = 0;
    context->send_ip_pkts = 0;
    context->log_file = NULL;
    context->filter_duplicates = 0;
    context->filter_unsuccessful = 0;
    context->max_sendto_failures = -1;
    context->min_hitrate = 0.0;
    context->recv_ready = 0;
    context->data_link_size = sizeof(struct ether_header);
    context->shm_fname = NULL;
    context->shm_fsize = 0;
    context->shm_fentry_size = 0;
    context->shm_flush_timeout = 0;

    context->xsend.start = 0.0;
    context->xsend.finish = 0.0;
    context->xsend.sent = 0;
    context->xsend.tried_sent = 0;
    context->xsend.blacklisted = 0;
    context->xsend.whitelisted = 0;
    context->xsend.warmup = 1;
    context->xsend.complete = 0;
    context->xsend.sendto_failures = 0;
    context->xsend.max_targets = 0;
    context->xsend.list_of_ips_pbm = NULL;

    context->xrecv.success_unique = 0;
    context->xrecv.success_total = 0;
    context->xrecv.app_success_unique = 0;
    context->xrecv.app_success_total = 0;
    context->xrecv.validation_passed = 0;
    context->xrecv.validation_failed = 0;
    context->xrecv.cooldown_unique = 0;
    context->xrecv.cooldown_total = 0;
    context->xrecv.failure_total = 0;
    context->xrecv.filter_success = 0;
    context->xrecv.ip_fragments = 0;
    context->xrecv.complete = 0;
    context->xrecv.pcap_recv = 0;
    context->xrecv.pcap_drop = 0;
    context->xrecv.pcap_ifdrop = 0;

}

static int _log_level_get(const char *log_level){

    int lv;

    if(log_level == NULL ||strlen(log_level)==0)
        return XM_LOG_INFO;

    if(strcasecmp(log_level,"info") == 0){
        lv = XM_LOG_INFO;
    }else if(strcasecmp(log_level,"debug") == 0){
        lv = XM_LOG_DEBUG;
    }else if(strcasecmp(log_level,"notice") == 0){
        lv = XM_LOG_NOTICE;
    }else if(strcasecmp(log_level,"warn") == 0){
        lv = XM_LOG_WARN;
    }else if(strcasecmp(log_level,"error") == 0){
        lv = XM_LOG_ERR;
    }else if(strcasecmp(log_level,"crit") == 0){
        lv = XM_LOG_CRIT;
    }else if(strcasecmp(log_level,"alert") == 0){
        lv = XM_LOG_ALERT;
    }else if(strcasecmp(log_level,"emerg") == 0){
        lv = XM_LOG_EMERG;
    }else {
        lv = XM_LOG_INFO;
    }

    return lv;
}

#define SET_IF_GIVEN(DST, ARG)                                                 \
	{                                                                      \
		if (args->ARG##_given) {                                        \
			(DST) = args->ARG##_arg;                                \
		};                                                             \
	}
#define SET_BOOL(DST, ARG)                                                     \
	{                                                                      \
		if (args->ARG##_given) {                                        \
			(DST) = 1;                                             \
		};                                                             \
	}

static int parse_mac(uint8_t *out, char *in)
{
    int i;

	if (strlen(in) < MAC_ADDR_LEN * 3 - 1)
		return 0;
	char octet[4];
	octet[2] = '\0';
	for (i = 0; i < MAC_ADDR_LEN; i++) {
		if (i < MAC_ADDR_LEN - 1 && in[i * 3 + 2] != ':') {
			return 0;
		}
		strncpy(octet, &in[i * 3], 2);
		char *err = NULL;
		long b = strtol(octet, &err, 16);
		if (err && *err != '\0') {
			return 0;
		}
		out[i] = b & 0xFF;
	}
	return 1;
}

static char* _interface_cfname_get(const char *type){

    if(strcmp(type,"pfring")==0){
        return "/usr/local/xmap/conf/if_pfring.conf";
    }else if(strcmp(type,"libpcap")==0){

        return "/usr/local/xmap/conf/if_pcap.conf";
    }

    return NULL;
}

static int  _context_set(xm_context_t *context,struct gengetopt_args_info *args){

    context->log_level = _log_level_get(args->log_level_arg);
    context->log_file = args->log_file_arg;

    if(context->log_file == NULL)
        context->log_file = "/tmp/xmap.log";

    /*init log*/
    xm_log_init(context->mp,context->log_file,context->log_level);

    context->probe = xm_probe_get_by_name(args->probe_module_arg);
    if(context->probe == NULL){

        xm_log(XM_LOG_ERR,"specified probe module (%s) does not exist!",args->probe_module_arg);

        return -1;
    }
	
    if (args->iplayer_given) {
		context->send_ip_pkts = 1;
		context->gw_mac_set = 1;
		memset(context->gw_mac, 0, MAC_ADDR_LEN);
	}
    
    context->filter_duplicates = 1;
	context->filter_unsuccessful = 1;
    
	context->ignore_invalid_hosts = args->ignore_invalid_hosts_given ||
				     args->ignore_blacklist_errors_given;

	
    context->cooldown_secs = args->cooldown_time_arg;
	SET_IF_GIVEN(context->blacklist_filename, blacklist_file);
	SET_IF_GIVEN(context->list_of_ips_filename, list_of_ips_file);
	SET_IF_GIVEN(context->probe_args, probe_args);
	SET_IF_GIVEN(context->probe_ttl, probe_ttl);
	SET_IF_GIVEN(context->max_runtime, max_runtime);
	SET_IF_GIVEN(context->max_results, max_results);
	SET_IF_GIVEN(context->rate, rate);
	SET_IF_GIVEN(context->packet_streams, probes);
	SET_IF_GIVEN(context->num_retries, retries);
	SET_IF_GIVEN(context->max_sendto_failures, max_sendto_failures);
	SET_IF_GIVEN(context->min_hitrate, min_hitrate);
	SET_IF_GIVEN(context->shm_fname, shm_fname);
	if(context->shm_fname == NULL)
        context->shm_fname = "/tmp/xmap_shm_file";

    SET_IF_GIVEN(context->shm_fentry_size,shm_fentry_size);
    if(context->shm_fentry_size<=0)
        context->shm_fentry_size = 65536;

    SET_IF_GIVEN(context->shm_fsize, shm_fsize);
	if(context->shm_fsize<=0)
        context->shm_fsize = 4294967296l; 

    SET_IF_GIVEN(context->shm_flush_timeout, shm_flush_timeout);
    if(context->shm_flush_timeout<=0)
        context->shm_flush_timeout = 60;

	SET_IF_GIVEN(context->whitelist_filename, whitelist_file);
	SET_IF_GIVEN(context->iface_send, interface_send);

    if(context->iface_send == NULL){
        context->iface_send = xm_get_default_iface();
        if(context->iface_send == NULL){
            xm_log(XM_LOG_ERR,"Please specified the send interface!");
            return -1;
        }
    }

	SET_IF_GIVEN(context->iface_recv, interface_recv);
    if(context->iface_recv == NULL){
        context->iface_recv = context->iface_send;
    }

    SET_IF_GIVEN(context->iface_send_type, interface_send_type);
    if(context->iface_send_type == NULL){

        context->iface_send_type = "linux";
    }

    SET_IF_GIVEN(context->iface_recv_type, interface_recv_type);
    if(context->iface_recv_type == NULL){
        context->iface_recv_type = "libpcap";
    }

    SET_IF_GIVEN(context->iface_send_cfname, interface_send_cfname);

    if(context->iface_send_cfname == NULL)
        context->iface_send_cfname = _interface_cfname_get(context->iface_send_type);

    SET_IF_GIVEN(context->iface_recv_cfname, interface_recv_cfname);
    if(context->iface_recv_cfname == NULL)
        context->iface_recv_cfname = _interface_cfname_get(context->iface_recv_type);

    if(args->core_mask_given)
        context->core_mask = args->core_mask_arg;
    else
        context->core_mask = "0xff";

    // find if xmap wants any specific cidrs scanned instead
	// of the entire Internet
	context->destination_cidrs = args->inputs;
	context->destination_cidrs_len = args->inputs_num;
    
	if (context->num_retries < 0) {
		xm_log(XM_LOG_ERR,"xmap Invalid retry count");
        return -1;
	}

	if (context->max_sendto_failures >= 0) {
		xm_log(XM_LOG_DEBUG,"xmap scan will abort if more than %i "
			  "sendto failures occur",
			  context->max_sendto_failures);
	}

	if (context->min_hitrate > 0.0) {
		xm_log(XM_LOG_DEBUG,"Xmap scan will abort if hitrate falls below %f",
			  context->min_hitrate);
	}
    
	if (context->probe->port_args) {
		if (args->source_port_given) {
			char *dash = strchr(args->source_port_arg, '-');
			if (dash) { // range
				*dash = '\0';
				context->source_port_first =
				    atoi(args->source_port_arg);
				
                xm_enforce_range("starting source-port",
					      context->source_port_first, 0,
					      0xFFFF);

				context->source_port_last = atoi(dash + 1);
				xm_enforce_range("ending source-port",
					      context->source_port_last, 0,
					      0xFFFF);

				if (context->source_port_first >
				    context->source_port_last) {
					fprintf(
					    stderr,
					    "%s: invalid source port range: "
					    "last port is less than first port\n",
					    CMDLINE_PARSER_PACKAGE);
					return -1;
				}

			} else { // single port
				int port = atoi(args->source_port_arg);
				xm_enforce_range("source-port", port, 0, 0xFFFF);
				context->source_port_first = port;
				context->source_port_last = port;
			}
		}

		if (!args->target_port_given) {
			xm_log(XM_LOG_ERR,
			    "xmap target port (-p) is required for this type of probe");
            return -1;
		}
		
        xm_enforce_range("target-port", args->target_port_arg, 0, 0xFFFF);
		context->target_port = args->target_port_arg;
	}
	
    if (args->source_ip_given) {
		xm_parse_source_ip_addresses(context,args->source_ip_arg);
	}

    if(context->number_source_ips == 0){

		struct in_addr default_ip;
		if (xm_get_iface_ip(context->iface_send, &default_ip) < 0) {
			xm_log(XM_LOG_ERR,"xmap could not detect default IP address for %s."
				  " Try specifying a source address (-S).",
				  context->iface_send);
            return -1;
		}

		context->source_ip_addresses[0] = default_ip.s_addr;
		context->number_source_ips++;
		xm_log(XM_LOG_DEBUG,
		    "Xmap no source IP address given. will use default address: %s.",
		    inet_ntoa(default_ip));
    }

	if (args->gateway_mac_given) {
		if (!parse_mac(context->gw_mac, args->gateway_mac_arg)) {
			fprintf(stderr, "%s: invalid MAC address `%s'\n",
				CMDLINE_PARSER_PACKAGE, args->gateway_mac_arg);
			return -1;
		}

		context->gw_mac_set = 1;
	}

    if(!context->gw_mac_set){

		struct in_addr gw_ip;
		memset(&gw_ip, 0, sizeof(struct in_addr));
		if (xm_get_default_gw(&gw_ip, context->iface_send) < 0) {
			xm_log(XM_LOG_ERR,
			    "xmap could not detect default gateway address for %s."
			    " Try setting default gateway mac address (-G).",
			    context->iface_send);
            return -1;
		}

		xm_log(XM_LOG_DEBUG,"xmap found gateway IP %s on %s", inet_ntoa(gw_ip),
			  context->iface_send);

		context->gw_ip = gw_ip.s_addr;
		memset(&context->gw_mac, 0, MAC_ADDR_LEN);

		if (xm_get_hw_addr(&gw_ip, context->iface_send, context->gw_mac)) {
			xm_log(XM_LOG_ERR,
			    "XMAP could not detect GW MAC address for %s on %s."
			    " Try setting default gateway mac address (-G), or run"
			    " \"arp <gateway_ip>\" in terminal.",
			    inet_ntoa(gw_ip), context->iface_send);
            return -1;
		}
		context->gw_mac_set = 1;
    }

	if (args->source_mac_given) {
		if (!parse_mac(context->hw_mac, args->source_mac_arg)) {
			fprintf(stderr, "%s: invalid MAC address `%s'\n",
				CMDLINE_PARSER_PACKAGE, args->gateway_mac_arg);
			return -1;
		}

		xm_log(XM_LOG_DEBUG,"send source MAC address specified on CLI: "
			  "%02x:%02x:%02x:%02x:%02x:%02x",
			  context->hw_mac[0], context->hw_mac[1], context->hw_mac[2],
			  context->hw_mac[3], context->hw_mac[4], context->hw_mac[5]);

		context->hw_mac_set = 1;
	}
	
    // Get the source hardware address, and give it to the probe
	// module
	if (!context->hw_mac_set) {
		if (xm_get_iface_hw_addr(context->iface_send, context->hw_mac)) {
			xm_log(XM_LOG_ERR,"xmap could not retrieve hardware address for "
				  "interface: %s",
				  context->iface_send);

			return -1;
		}

		xm_log(XM_LOG_DEBUG,
		    "no source MAC provided. "
		    "automatically detected %02x:%02x:%02x:%02x:%02x:%02x as hw "
		    "interface for %s",
		    context->hw_mac[0], context->hw_mac[1], context->hw_mac[2],
		    context->hw_mac[3], context->hw_mac[4], context->hw_mac[5],
		    context->iface_send);
        context->hw_mac_set =1;
	}
	
    // Check for a random seed
	if (args->seed_given) {
		context->seed = args->seed_arg;
		context->seed_provided = 1;
	} else {
		// generate a seed randomly
		if (!xm_random_bytes(&context->seed, sizeof(uint64_t))) {
			xm_log(XM_LOG_ERR,"xmap unable to generate random bytes "
					  "needed for seed");
            return -1;
		}

		context->seed_provided = 0;
	}
	
    context->aes = xm_aesrand_create_from_seed(context->mp,context->seed);
    if(context->aes==NULL){

        xm_log(XM_LOG_ERR,"Cannot crate aes failed!");
        return -1;
    }

	// Set up sharding
	context->shard_num = 0;
	context->total_shards = 1;

	if ((args->shard_given || args->shards_given) && !args->seed_given) {
		xm_log(XM_LOG_ERR,"xmap Need to specify seed if sharding a scan");
        return -1;
	}

	if (args->shard_given ^ args->shards_given) {
		xm_log(XM_LOG_ERR,
		    "xmap Need to specify both shard number and total number of shards");
        return -1;
	}

	if (args->shard_given) {
		xm_enforce_range("shard", args->shard_arg, 0, 65534);
	}

	if (args->shards_given) {
		xm_enforce_range("shards", args->shards_arg, 1, 65535);
	}

	SET_IF_GIVEN(context->shard_num, shard);
	SET_IF_GIVEN(context->total_shards, shards);
	
    if (context->shard_num >= context->total_shards) {
		xm_log(XM_LOG_ERR,"xmap With %hhu total shards, shard number (%hhu)"
			  " must be in range [0, %hhu)",
			context->total_shards, context->shard_num,
			  context->total_shards);
        return -1;
    }

	if (args->bandwidth_given) {
		// Supported: G,g=*1000000000; M,m=*1000000 K,k=*1000 bits per
		// second
		context->bandwidth = atoi(args->bandwidth_arg);
		char *suffix = args->bandwidth_arg;
		while (*suffix >= '0' && *suffix <= '9') {
			suffix++;
		}
		if (*suffix) {
			switch (*suffix) {
			case 'G':
			case 'g':
				context->bandwidth *= 1000000000;
				break;
			case 'M':
			case 'm':
				context->bandwidth *= 1000000;
				break;
			case 'K':
			case 'k':
				context->bandwidth *= 1000;
				break;
			default:
				fprintf(stderr,
					"%s: unknown bandwidth suffix '%s' "
					"(supported suffixes are G, M and K)\n",
					CMDLINE_PARSER_PACKAGE, suffix);
				return -1;
			}
		}
	}

	if (args->max_targets_given) {
		context->max_targets = xm_parse_max_hosts(args->max_targets_arg);
	}

    context->wblist = xm_wblist_create(context->mp,context->whitelist_filename,
            context->blacklist_filename,
            context->destination_cidrs,
            context->destination_cidrs_len,
            NULL,0,
            context->ignore_invalid_hosts);

    if(context->wblist == NULL){

        xm_log(XM_LOG_ERR,"xmap unable to initialize blacklist / whitelist");
        return -1;
    }

	// if there's a list of ips to scan, then initialize PBM and populate
	// it based on the provided file
	if (context->list_of_ips_filename) {
		
        context->xsend.list_of_ips_pbm = xm_pbm_init();

		context->list_of_ips_count = xm_pbm_load_from_file(
		    context->xsend.list_of_ips_pbm, context->list_of_ips_filename);
	}

	// compute number of targets
	uint64_t allowed = xm_wblist_count_allowed(context->wblist);
	context->total_allowed = allowed;
	context->total_disallowed = xm_wblist_count_not_allowed(context->wblist);

    if(allowed > (1LL << 32)){

        return -1;
    }

	if (!context->total_allowed) {
		xm_log(XM_LOG_ERR,"Xmap zero eligible addresses to scan");
        return -1;
	}

	if (context->max_targets) {
		context->xsend.max_targets = context->max_targets;
	}

	// Set the correct number of threads, default to num_cores - 1
	if (args->sender_threads_given) {
		context->senders = args->sender_threads_arg;
	} else {
		context->senders = 1;
	}
	if (2 * context->senders >= context->xsend.max_targets) {
		xm_log(XM_LOG_WARN,
		    "xmap too few targets relative to senders, dropping to one sender");
        context->senders = 1;
	}

    context->send_interface = xm_if_create(context->mp,context->iface_send,context->iface_send_type,context->iface_send_cfname); 
    if(context->send_interface == NULL){

        xm_log(XM_LOG_ERR,"Cannot create send interface:%s,%s",context->iface_send,context->iface_send_type);
        return -1;
    }
    
    context->recv_interface = xm_if_create(context->mp,context->iface_recv,context->iface_recv_type,context->iface_recv_cfname); 
    if(context->recv_interface == NULL){

        xm_log(XM_LOG_ERR,"Cannot create recv interface:%s,%s",context->iface_recv,context->iface_recv_type);
        return -1;
    }

    context->probe->probe_context_init(context);

    pthread_mutex_init(&context->recv_ready_mutex,NULL);

    xm_validate_init();

    return 0;
}

xm_context_t * xm_context_create(xm_pool_t *mp,struct gengetopt_args_info *args){

    xm_context_t *context = (xm_context_t*)xm_pcalloc(mp,sizeof(*context));
    context->mp = mp;

    _context_init(context);

    if(_context_set(context,args)){

        xm_log(XM_LOG_ERR,"Create context failed!");

        return NULL;
    }


    return context;
}

