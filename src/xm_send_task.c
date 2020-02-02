/*
 * =====================================================================================
 *
 *       Filename:  xm_send_task.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/02/2019 11:33:56 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jacksha (shajf), csp001314@163.com
 *   Organization:  
 *
 * =====================================================================================
 */
#include "xm_send_task.h"
#include "xm_log.h"
#include "xm_validate.h"
#include "xm_pbm.h"
#include "xm_util.h"
// Lock for send run

static pthread_mutex_t send_mutex = PTHREAD_MUTEX_INITIALIZER;

static int _send_task_init(xm_task_t *task xm_unused,void *priv_data xm_unused){

	return 0;
}

static int _send_task_exit(xm_task_t *task xm_unused,void *priv_data xm_unused){

	return 0;
}

static inline uint32_t get_src_ip(xm_context_t *context,uint32_t dst, int local_offset)
{
	if (context->number_source_ips == 1) {
		return context->source_ip_addresses[0];
	}

	return context->source_ip_addresses[(ntohl(dst) + context->srcip_offset + local_offset) %
				 context->number_source_ips];
}

static int _send_task_run(xm_task_t *task,void *priv_data xm_unused){


    xm_send_task_t *send_task = (xm_send_task_t*)task;
	xm_context_t *context = send_task->context;
	xm_shard_t *shard = send_task->shard; 

    char buf[MAX_PACKET_SIZE];
	memset(buf, 0, MAX_PACKET_SIZE);
	
    pthread_mutex_lock(&send_mutex);
    void *probe_data;

    if(context->probe->probe_thread_init){
        context->probe->probe_thread_init(buf,context->hw_mac,context->gw_mac,context->target_port,&probe_data);
    }

	pthread_mutex_unlock(&send_mutex);
	
    // adaptive timing to hit target rate
	uint64_t count = 0;
	uint64_t last_count = count;
	double last_time = now();
	uint32_t delay = 0;
	int interval = 0;
	volatile int vi;
	struct timespec ts, rem;
	double send_rate = (double)context->rate /context->senders;
	const double slow_rate = 50; // packets per seconds per thread
	// at which it uses the slow methods
	long nsec_per_sec = 1000 * 1000 * 1000;
	long long sleep_time = nsec_per_sec;
	if (context->rate > 0) {
		delay = 10000;
		if (send_rate < slow_rate) {
			// set the inital time difference
			sleep_time = nsec_per_sec / send_rate;
			last_time = now() - (1.0 / send_rate);
		} else {
			// estimate initial rate
			for (vi = delay; vi--;)
				;
			delay *= 1 / (now() - last_time) /
				 (context->rate / context->senders);
			interval = (context->rate / context->senders) / 20;
			last_time = now();
		}
	}
	
		// Get the initial IP to scan.
	uint32_t current_ip = xm_shard_get_cur_ip(shard);

	// If provided a list of IPs to scan, then the first generated address
	// might not be on that list. Iterate until the current IP is one the
	// list, then start the true scanning process.
	if (context->list_of_ips_filename) {
		while (!xm_pbm_check(context->xsend.list_of_ips_pbm, current_ip)) {
			current_ip = xm_shard_get_next_ip(shard);

			shard->state.tried_sent++;
			if (current_ip == XM_SHARD_DONE) {
				xm_log(XM_LOG_DEBUG,
				    "send never made it to send loop in send thread %i",
				    send_task->thread_id);
				goto cleanup;
			}
		}
	}

	int attempts = context->num_retries + 1;
	uint32_t idx = 0;
	while (1) {
		// Adaptive timing delay
		send_rate = (double)context->rate / context->senders;
		if (count && delay > 0) {
			if (send_rate < slow_rate) {
				double t = now();
				double last_rate = (1.0 / (t - last_time));

				sleep_time *= ((last_rate / send_rate) + 1) / 2;
				ts.tv_sec = sleep_time / nsec_per_sec;
				ts.tv_nsec = sleep_time % nsec_per_sec;
				xm_log(XM_LOG_DEBUG,"sleep sleep for %d sec, %ld nanoseconds",
					  ts.tv_sec, ts.tv_nsec);

				while (nanosleep(&ts, &rem) == -1) {
				}
				last_time = t;
			} else {
				for (vi = delay; vi--;)
					;
				if (!interval || (count % interval == 0)) {
					double t = now();
					double multiplier =
					    (double)(count - last_count) /
					    (t - last_time) /
					    (context->rate / context->senders);
					uint32_t old_delay = delay;
					delay *= multiplier;
					if (delay == old_delay) {
						if (multiplier > 1.0) {
							delay *= 2;
						} else if (multiplier < 1.0) {
							delay *= 0.5;
						}
					}
					last_count = count;
					last_time = t;
				}
			}
		}

		// Check all the ways a send thread could finish and break out
		// of the send loop.
		if (context->xrecv.complete) {
			break;
		}

		if (shard->state.max_targets &&
		    (shard->state.sent >= shard->state.max_targets ||
		     shard->state.tried_sent >= shard->state.max_targets)) {
			xm_log(XM_LOG_DEBUG,"send thread %d finished (max targets of %u reached)",
			send_task->thread_id, shard->state.max_targets);

			break;
		}

		if (context->max_runtime &&
		    context->max_runtime <= now() - context->xsend.start) {
			break;
		}

		if (current_ip == XM_SHARD_DONE) {
			xm_log(XM_LOG_DEBUG,"send thread %d finished, shard depleted",
				  send_task->thread_id);
			break;
		}

        int i;
		// Actually send a packet.
		for (i = 0; i < context->packet_streams; i++) {
			count++;
			uint32_t src_ip = get_src_ip(context,current_ip, i);
			uint32_t validation[VALIDATE_BYTES / sizeof(uint32_t)];

			xm_validate_gen(src_ip, current_ip, (uint8_t *)validation);

			uint8_t ttl = context->probe_ttl;

			size_t length = context->probe->packet_length;

			context->probe->probe_make_packet(buf, &length, src_ip,
							current_ip, ttl, validation,
							i, probe_data);

			if (length > MAX_PACKET_SIZE) {
				xm_log(XM_LOG_ERR,
				    "send thread %d set length (%zu) larger than MAX (%zu)",
				    send_task->thread_id, length, MAX_PACKET_SIZE);
				break;
			}

			void *contents =
				buf + context->send_ip_pkts *
					  sizeof(struct ether_header);
			length -= (context->send_ip_pkts *
				   sizeof(struct ether_header));
			int any_sends_successful = 0;
            int j;
			for (j = 0; j < attempts; ++j) {
				int rc = send_task->if_driver->send(send_task->if_driver, contents,
							 length, idx,NULL);
				if (rc < 0) {
					struct in_addr addr;
					addr.s_addr = current_ip;
					char addr_str_buf
						[INET_ADDRSTRLEN];
					const char *addr_str =
						inet_ntop(AF_INET, &addr,
							  addr_str_buf,
							  INET_ADDRSTRLEN);
					if (addr_str != NULL) {
						xm_log(XM_LOG_DEBUG,"send_packet failed for %s. %s",
							addr_str,
							strerror(errno));
					}

				} else {
					any_sends_successful = 1;
					break;
				}
			}
			if (!any_sends_successful) {
				shard->state.failures++;
			}
			idx++;
			idx &= 0xFF;
		}
		// Track the number of hosts we actually scanned.
		shard->state.sent++;
		shard->state.tried_sent++;

		// Get the next IP to scan
		current_ip = xm_shard_get_next_ip(shard);
		if (context->list_of_ips_filename &&
		    current_ip != XM_SHARD_DONE) {
			// If we have a list of IPs bitmap, ensure the next IP
			// to scan is on the list.
			while (!xm_pbm_check(context->xsend.list_of_ips_pbm, current_ip)) {
				current_ip = xm_shard_get_next_ip(shard);
				shard->state.tried_sent++;
				if (current_ip == XM_SHARD_DONE) {
					xm_log(XM_LOG_DEBUG,
					    "send thread %d shard finished in get_next_ip_loop depleted",
					    send_task->thread_id);
					goto cleanup;
				}
			}
		}
	}//end while(1)

cleanup:
	xm_log(XM_LOG_DEBUG,"thread %d cleanly finished", send_task->thread_id);

	return TASK_RETURN_EXIT;
}

static xm_shard_t* _create_shard(xm_context_t *context,uint8_t thread_idx){
	
	uint64_t num_addrs = xm_wblist_count_allowed(context->wblist);
	xm_cyclic_group_t *group = xm_get_cyclic_group(num_addrs);
	if(num_addrs>(1LL<<32)){
		context->xsend.max_index = 0xFFFFFFFF;
	}else{
		context->xsend.max_index = (uint32_t)num_addrs;
	}

	xm_cycle_t *cycle = xm_make_cycle(context->mp,group,context->aes);

	
	return xm_shard_create(context->mp,context->shard_num,context->total_shards,thread_idx,
			   context->senders,context->xsend.max_targets,context->xsend.max_index,cycle,context->wblist);
	
}

xm_send_task_t * xm_send_task_create(xm_context_t *context,int thread_id) {
	
	xm_send_task_t *send_task = xm_pcalloc(context->mp,sizeof(*send_task));
	send_task->task.tsk_id = (uint32_t)thread_id;
	send_task->task.run = _send_task_run;
	send_task->task.init = _send_task_init;
	send_task->task.exit = _send_task_exit;

	send_task->context = context;
	send_task->thread_id = thread_id;

	send_task->shard = _create_shard(context,(uint8_t)thread_id);
	if(send_task->shard == NULL){
		xm_log(XM_LOG_ERR,"Create thread:%d  shard failed!",thread_id);
		return NULL;
	} 
    context->send_interface->if_driver->init(context->send_interface->if_driver,NULL);
	send_task->if_driver = context->send_interface->if_driver->create_tx_private_data(context->send_interface->if_driver,0,NULL); 

	if(send_task->if_driver == NULL){
		xm_log(XM_LOG_ERR,"Cannot create send interface failed!");
		return NULL;
	}

	return send_task;
}

