
#include "xm_ip_iterator.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "xm_wblist.h"
#include "xm_random.h"
#include "xm_shard.h"
#include "xm_validate.h"
#include "xm_jni_util.h"
#include "xm_log.h"

typedef struct java_xit_context_t java_xit_context_t;
typedef struct java_xit_conf java_xit_conf;

struct java_xit_conf {
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

struct java_xit_context_t {

    java_xit_conf conf;
    xm_pool_t *mp;
    xm_shard_t *shard;
    uint32_t cur_ip;
};

static java_xit_context_t xit_context,*xit_context_ptr = &xit_context;

static xm_shard_t* _create_shard(java_xit_conf *conf){
	
	uint64_t num_addrs = xm_wblist_count_allowed(conf->wblist);

	xm_cyclic_group_t *group = xm_get_cyclic_group(num_addrs);
	xm_cycle_t *cycle = xm_make_cycle(conf->mp,group,conf->aes);

	
	return xm_shard_create(conf->mp,conf->shard_num, conf->total_shards,0,
			   1,num_addrs,num_addrs,cycle,conf->wblist);
	
}


static int _prepare(const char *wlist_path,const char *blist_path){


	memset(xit_context_ptr, 0, sizeof(struct java_xit_context_t));

    java_xit_conf *conf = &xit_context_ptr->conf;
	conf->ignore_errors = 1;
    conf->log_filename = "/tmp/xiterate.log";
    conf->verbosity = XM_LOG_DEBUG;

    xit_context_ptr->mp = xm_pool_create(1024);
    conf->mp = xit_context_ptr->mp;
    xm_log_init(conf->mp,conf->log_filename,conf->verbosity);

	// Blacklist and whitelist
	if (blist_path!=NULL&&strlen(blist_path)>0) {
		conf->blacklist_filename = blist_path;
	}
	if (wlist_path!=NULL&&strlen(wlist_path)>0) {
		conf->whitelist_filename = wlist_path;
	}

    conf->max_hosts = 0xFFFFFFFF;

	// parse blacklist and whitelist
    xm_wblist_t *wblist = xm_wblist_create(conf->mp,conf->whitelist_filename,
	conf->blacklist_filename, conf->destination_cidrs, conf->destination_cidrs_len,
	NULL,0,conf->ignore_errors);

	if (wblist == NULL) {
        xm_pool_destroy(xit_context_ptr->mp);
        return -1;
	}

    conf->wblist = wblist;

	// Set up sharding
	conf->shard_num = 0;
	conf->total_shards = 1;

	if (!xm_random_bytes(&conf->seed, sizeof(uint64_t))) {
        
        xm_pool_destroy(xit_context_ptr->mp);
		return -1;
	}

	conf->aes = xm_aesrand_create_from_seed(conf->mp,conf->seed);

	xm_shard_t *shard = _create_shard(conf);
    xit_context_ptr->shard = shard;
	xit_context_ptr->cur_ip = xm_shard_get_cur_ip(shard);

	return 0;
}

static long get_next(){

    long cur = xit_context_ptr->cur_ip;
	xit_context_ptr->cur_ip = xm_shard_get_next_ip(xit_context_ptr->shard);

    return cur;
}

static int _do_gen(const char *wlist_path,const char *blist_path,const char *out_path,const char *ports){

    java_xit_conf conf;
    xm_pool_t *mp;
    FILE *fp;

    fp = fopen(out_path,"w+");

    if(fp == NULL)
        return -1;

	memset(&conf, 0, sizeof(struct java_xit_conf));
	conf.ignore_errors = 1;
    conf.log_filename = "/tmp/xiterate.log";
    conf.verbosity = XM_LOG_DEBUG;

    mp = xm_pool_create(1024);
    conf.mp = mp;
    xm_log_init(mp,conf.log_filename,conf.verbosity);

	// Blacklist and whitelist
	if (blist_path!=NULL&&strlen(blist_path)>0) {
		conf.blacklist_filename = blist_path;
	}
	if (wlist_path!=NULL&&strlen(wlist_path)>0) {
		conf.whitelist_filename = wlist_path;
	}

    conf.max_hosts = 0xFFFFFFFF;

	// parse blacklist and whitelist
    xm_wblist_t *wblist = xm_wblist_create(mp,conf.whitelist_filename,
	conf.blacklist_filename, conf.destination_cidrs, conf.destination_cidrs_len,
	NULL,0,conf.ignore_errors);

	if (wblist == NULL) {
        fclose(fp);
        xm_pool_destroy(mp);
        return -1;
	}

    conf.wblist = wblist;

	// Set up sharding
	conf.shard_num = 0;
	conf.total_shards = 1;

	if (!xm_random_bytes(&conf.seed, sizeof(uint64_t))) {
        
        fclose(fp);
        xm_pool_destroy(mp);
		return -1;
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
		fprintf(fp,"%s:%s\n", inet_ntoa(next_ip),ports);
		next_int = xm_shard_get_next_ip(shard);
	}

    xm_pool_destroy(mp);
    fclose(fp);
	return count;

}

/*
 * Class:     com_xmap_api_XMapIPIterator
 * Method:    iterate
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)I
 */
JNIEXPORT jint JNICALL Java_com_xmap_api_XMapIPIterator_iterate__Ljava_lang_String_2Ljava_lang_String_2Ljava_lang_String_2I
  (JNIEnv *jenv, jobject jthis, jstring wlistPath, jstring blistPath, jstring outPath, jint port){

    char b[64] = {0};

    const char *wlist_path = xm_string_arg_get(jenv,wlistPath);
    const char *blist_path = xm_string_arg_get(jenv,blistPath);
    const char *out_path = xm_string_arg_get(jenv,outPath);

    snprintf(b,64,"%d",port);

    return _do_gen(wlist_path,blist_path,out_path,b);
}

/*
 * Class:     com_xmap_api_XMapIPIterator
 * Method:    iterate
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_xmap_api_XMapIPIterator_iterate__Ljava_lang_String_2Ljava_lang_String_2Ljava_lang_String_2Ljava_lang_String_2
(JNIEnv *jenv, jobject jthis, jstring wlistPath, jstring blistPath, jstring outPath, jstring ports){

    const char *wlist_path = xm_string_arg_get(jenv,wlistPath);
    const char *blist_path = xm_string_arg_get(jenv,blistPath);
    const char *out_path = xm_string_arg_get(jenv,outPath);
    const char *portlist = xm_string_arg_get(jenv,ports);

    return _do_gen(wlist_path,blist_path,out_path,portlist);


}

/*
 * Class:     com_xmap_api_XMapIPIterator
 * Method:    prepareIterate
 * Signature: (Ljava/lang/String;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_xmap_api_XMapIPIterator_prepareIterate
(JNIEnv *jenv, jobject jthis, jstring wlistPath, jstring blistPath){


    const char *wlist_path = xm_string_arg_get(jenv,wlistPath);
    const char *blist_path = xm_string_arg_get(jenv,blistPath);

    return _prepare(wlist_path,blist_path); 
}

/*
 * Class:     com_xmap_api_XMapIPIterator
 * Method:    getNextIP
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_com_xmap_api_XMapIPIterator_getNextIP
(JNIEnv *jenv, jobject jthis){

    return get_next();
}

/*
 * Class:     com_xmap_api_XMapIPIterator
 * Method:    closeIterate
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_xmap_api_XMapIPIterator_closeIterate
(JNIEnv *jenv, jobject jthis){

    xm_pool_destroy(xit_context_ptr->mp);
}
