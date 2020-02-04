/*
 * =====================================================================================
 *
 *       Filename:  xm_entry_reader.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/17/2019 04:12:12 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *   Organization:  
 *
 * =====================================================================================
 */
#include "xm_entry_reader.h"
#include "xm_jni_util.h"
#include "xm_mpool.h"
#include "xm_shm_format.h"
#include "xm_shm_entry_record.h"

#define MAX_ENTRY_RCD_CONTEXT_N 64

typedef struct xm_shm_entry_record_reader_context_t xm_shm_entry_record_reader_context_t;


struct xm_shm_entry_record_reader_context_t {
	xm_pool_t *mp;
	xm_shm_format_t *shm_fmt;
	xm_shm_entry_iterator_t *eiter;
};

static xm_shm_entry_record_reader_context_t entry_rcd_contexts[MAX_ENTRY_RCD_CONTEXT_N];

static inline xm_shm_entry_record_reader_context_t * _entry_rcd_context_get(int id){

	if(id >= MAX_ENTRY_RCD_CONTEXT_N||id<0)
		return NULL;

	return &entry_rcd_contexts[id];
}


/*
 * Class:     com_xmap_api_XMapEntryReader
 * Method:    openMMap
 * Signature: (ILjava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_xmap_api_XMapEntryReader_openMMap
(JNIEnv *jenv, jobject jthis, jint id, jstring jfname){

	  xm_shm_entry_record_reader_context_t *pcontext;
	  xm_shm_format_t *shm_fmt;
	  xm_pool_t *mp;
	  const char *open_fname;
	  
	  pcontext = _entry_rcd_context_get(id);
	  if(pcontext == NULL)
		  return -1;

	  open_fname = xm_string_arg_get(jenv,jfname);
	  if(open_fname == NULL || strlen(open_fname) == 0)
		  return -1;
	  /*create global apr memory pool*/
	  mp = xm_pool_create(1024);
	  if(mp == NULL){
		  return -1;
	  }
	  
	  shm_fmt = xm_shm_format_with_mmap_create(mp,open_fname,0,0,0,0);
	  if(shm_fmt == NULL)
		  return -1;


	  pcontext->mp = mp;

	  pcontext->shm_fmt = shm_fmt;

	  pcontext->eiter = NULL;

    
	  return 0;

}

/*
 * Class:     com_xmap_api_XMapEntryReader
 * Method:    openSHM
 * Signature: (ILjava/lang/String;I)I
 */
JNIEXPORT jint JNICALL Java_com_xmap_api_XMapEntryReader_openSHM
(JNIEnv *jenv, jobject jthis, jint id, jstring key, jint proj_id){

	  xm_shm_entry_record_reader_context_t *pcontext;
	  xm_shm_format_t *shm_fmt;
	  xm_pool_t *mp;
	  
	  const char *open_key;
	  
	  pcontext = _entry_rcd_context_get(id);
	  if(pcontext == NULL)
		  return -1;
	  
	  open_key = xm_string_arg_get(jenv,key);

	  if(open_key == NULL|| strlen(open_key) == 0)
		  return -1;
	  /*create global apr memory pool*/
	  
	  mp = xm_pool_create(1024);
	  if(mp == NULL){
		  return -1;
	  }
	  
	  shm_fmt = xm_shm_format_with_shm_create(mp,open_key,proj_id,0,0,0,0);
	  if(shm_fmt == NULL)
		  return -1;
	  
	  pcontext->mp = mp;

	  pcontext->shm_fmt = shm_fmt;

	  pcontext->eiter = NULL;
	  
	  return 0;

}

static inline int _prepare_iter_ok(xm_shm_entry_record_reader_context_t *pcontext){

	if(pcontext == NULL)
		return 0;

    if(pcontext->eiter)
		return 1;

    pcontext->eiter = xm_shm_entry_iterator_prefare(pcontext->shm_fmt);

    if(pcontext->eiter == NULL)
		return 0;
     
    return 1;
}

/*
 * Class:     com_xmap_api_XMapEntryReader
 * Method:    read
 * Signature: (ILcom/xmap/api/XMapEntry;)I
 */
JNIEXPORT jint JNICALL Java_com_xmap_api_XMapEntryReader_read
(JNIEnv *jenv, jobject jthis, jint id, jobject jentry){

	  xm_shm_entry_record_reader_context_t *pcontext = NULL;
	  xm_shm_format_t *shm_fmt;
	  xm_shm_entry_iterator_t *eiter;

	  xm_shm_entry_record_t *entry_rcd;
	  xm_shm_record_t *shm_rcd;
	  
	  pcontext = _entry_rcd_context_get(id);
	  if(pcontext == NULL)
		  return -1;

     if(_prepare_iter_ok(pcontext) == 0){
		 /* no data to be read */
		 return -2;
     }

	 shm_fmt = pcontext->shm_fmt;
	 eiter = pcontext->eiter;

     shm_rcd = eiter->next(eiter);

	 if(shm_rcd == NULL){
		 /* read one data chunk completely*/
		 xm_shm_entry_iterator_commit(shm_fmt,eiter);

		 pcontext->eiter = NULL;
		 return -1;
	 } 

	 entry_rcd = (xm_shm_entry_record_t*)shm_rcd;
     /* read ok */
	 
	 xm_set_int_field(jenv,jentry,"type",(int)entry_rcd->type);
	 xm_set_int_field(jenv,jentry,"dataSize",(int)shm_rcd->data_len);

	 if(shm_rcd->data_len>0)
		 xm_send_data_to_java(jenv,jentry,shm_rcd->data,(long)shm_rcd->data_len);

	return 0;

}

/*
 * Class:     com_xmap_api_XMapEntryReader
 * Method:    close
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_xmap_api_XMapEntryReader_close
(JNIEnv *jenv, jobject jthis, jint id){

      xm_shm_entry_record_reader_context_t *pcontext = NULL;

	  pcontext = _entry_rcd_context_get(id);
	  if(pcontext == NULL)
		  return;

	  if(pcontext->eiter){
	  
		  xm_shm_entry_iterator_commit(pcontext->shm_fmt,pcontext->eiter);
	  }

	  xm_pool_destroy(pcontext->mp);

}

