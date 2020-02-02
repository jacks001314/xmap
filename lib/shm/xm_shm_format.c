/*
 * =====================================================================================
 *
 *       Filename:  xm_shm_format.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  11/01/2017 04:31:36 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  shajf (jacks), csp001314@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */
#include "xm_log.h"
#include "xm_shm_format.h"

xm_shm_format_t * xm_shm_format_create(xm_pool_t *mp,xm_shm_interface_t *shm_int,
  uint32_t entry_size,
  uint32_t record_header_size,
  xm_shm_record_t *shm_record,
  void (*shm_record_write)(xm_shm_format_t *fmt,xm_shm_record_t *record),
  int (*shm_record_read)(xm_shm_entry_iterator_t *iter,xm_bin_format_t *bfmt,xm_shm_record_t *shm_record)){


   xm_shm_format_t *fmt = NULL;


   fmt = (xm_shm_format_t*)xm_pcalloc(mp,sizeof(xm_shm_format_t));
   fmt->mp = mp;
   fmt->shm_int = shm_int;
   fmt->has_header = 0;
   fmt->entry_size = entry_size;
   fmt->record_header_size = record_header_size;
   fmt->shm_record_write = shm_record_write;
   fmt->shm_record_read = shm_record_read;

   fmt->shm_record = shm_record;

   return fmt;
}

static inline int _is_record_size_over(xm_shm_format_t *fmt,xm_shm_record_t *record){

    uint32_t rem_entry_size = fmt->entry_size-XM_SHM_ENTRY_HEADER_LEN;
    uint32_t record_size = (uint32_t)record->record_size;

    return record_size > rem_entry_size;
}

static inline int _is_need_create_entry_header(xm_shm_format_t *fmt,xm_shm_record_t *record){

    xm_bin_format_t *bfmt = &fmt->cur_header.bfmt;
    
    if(fmt->has_header == 0)
        return 1;

    if(xm_bf_full(bfmt,(size_t)record->record_size))
        return 1;

    return 0;
}

static inline void _shm_entry_header_commit(xm_shm_format_t *fmt,xm_shm_entry_header_t *header){

    xm_bin_format_t *bfmt = &header->bfmt;
    xm_shm_entry_t *shm_entry = &header->shm_entry;
    xm_bf_reset(bfmt);

    xm_bf_uint32_write(bfmt,header->magic);
    xm_bf_uint32_write(bfmt,header->major_version);
    xm_bf_uint32_write(bfmt,header->minor_version);
    xm_bf_uint32_write(bfmt,header->entries_number);
    xm_bf_uint32_write(bfmt,header->entries_size);

    xm_shm_entry_commit(fmt->shm_int,shm_entry);
}

static inline void _shm_entry_header_init(xm_shm_entry_header_t *header){

    xm_bin_format_t *bfmt = &header->bfmt;
    xm_bf_init(bfmt,header->shm_entry.start,header->shm_entry.end);

    header->magic = FIMAGIC;
    header->major_version = MAJOR_VERSION;
    header->minor_version = MINJOR_VERSION;
    header->entries_size = 0;
    header->entries_number = 0;

    xm_bf_skip(bfmt,XM_SHM_ENTRY_HEADER_LEN);
}

static int _shm_entry_header_create(xm_shm_format_t *fmt){

    if(fmt->has_header){
        _shm_entry_header_commit(fmt,&fmt->cur_header);
        fmt->has_header = 0;
    }

    if(-1 == xm_shm_entry_get(fmt->shm_int,&(fmt->cur_header.shm_entry))){
        return -1;
    }

    fmt->has_header = 1;
    _shm_entry_header_init(&fmt->cur_header);
    /*ok*/
    return 0;
}

static void _shm_record_write(xm_shm_format_t *fmt,xm_shm_record_t *record){

    unsigned char *wdata;

    xm_shm_entry_header_t *header = &fmt->cur_header;
    xm_bin_format_t *bfmt = &header->bfmt;

    uint32_t bsize = (uint32_t)xm_bf_content_size(bfmt);

    xm_bf_uint32_write(bfmt,EIMAGIC);
    xm_bf_uint32_write(bfmt,xm_shm_record_len(fmt,record));

    fmt->shm_record_write(fmt,record);
    
    xm_bf_uint32_write(bfmt,record->data_offset);

    wdata = record->data == NULL ||record->data_len == 0?"":record->data;

    xm_bf_bytes_write(bfmt,wdata,record->data_len);

    uint32_t asize = (uint32_t)xm_bf_content_size(bfmt);

    header->entries_number+=1;
    header->entries_size += (asize-bsize);
}

int xm_shm_format_put(xm_shm_format_t *fmt,xm_shm_record_t *record){

   if(_is_record_size_over(fmt,record)){
   
        xm_log(XM_LOG_ERR,"The record size:%lu is too large!",record->record_size);
        return -2;
   }

    if(_is_need_create_entry_header(fmt,record)){

        if(-1 == _shm_entry_header_create(fmt)){

            xm_log(XM_LOG_ERR,"Cannot create a shm entry to store record!");
            return -1;
        }
    }

    _shm_record_write(fmt,record);

    /* ok */
    return 0;
}

static inline int _can_been_flush(xm_shm_format_t *fmt){

    xm_bin_format_t *bfmt = &fmt->cur_header.bfmt;
    if(fmt->has_header == 0)
       return 0;
    
    if(xm_bf_empty(bfmt))
       return 0;
    
    return 1;
}

void xm_shm_format_flush(xm_shm_format_t *fmt){
   
   if(_can_been_flush(fmt)){
      
      if(-1 == _shm_entry_header_create(fmt)){
         fmt->has_header = 0;
      
      }
   }
}

static inline int _shm_entry_header_read(xm_shm_entry_header_t *header,xm_bin_format_t *bfmt){

    header->magic = xm_bf_uint32_read(bfmt);
    if(header->magic!=FIMAGIC){
        xm_log(XM_LOG_ERR,"Shm entry header Magic:%lu is invalid!",header->magic);
        return -1;
    }

    header->major_version = xm_bf_uint32_read(bfmt);
    header->minor_version = xm_bf_uint32_read(bfmt);
    header->entries_number = xm_bf_uint32_read(bfmt);
    header->entries_size = xm_bf_uint32_read(bfmt);

    return 0;
}

static inline int has_next(xm_shm_entry_iterator_t *iter){

    if(iter->header == NULL)
        return 0;

    if(iter->entries_count>=iter->header->entries_number)
        return 0;

    return 1;
}

static inline int  _record_read(xm_shm_entry_iterator_t *iter,xm_bin_format_t *bfmt,xm_shm_record_t *record){

    uint32_t magic;
    uint32_t record_size;

    magic = xm_bf_uint32_read(bfmt);
    
    if(magic!=EIMAGIC){
        xm_log(XM_LOG_ERR,"record magic:%lu is invalid!",magic);
        iter->header = NULL;
        return -1;
    }
    
    record_size = xm_bf_uint32_read(bfmt);
    if(iter->fmt->shm_record_read(iter,bfmt,record)){
	
		/*error*/
		return -1;
	}
    
    record->magic = magic;
    record->record_size = record_size;
    record->data_offset = xm_bf_uint32_read(bfmt);

    xm_bf_bytes_read(bfmt,(unsigned char**)(&record->data),&record->data_len);

    return 0;
}

static xm_shm_record_t * record_next(xm_shm_entry_iterator_t *iter){

	int rc;

    xm_shm_record_t *record = iter->fmt->shm_record;
    xm_shm_entry_header_t *header = iter->header;

    xm_bin_format_t *bfmt = &header->bfmt;

    if(has_next(iter) == 0){
        return NULL;
    }

    rc =  _record_read(iter,bfmt,record);

    if(rc == 0){

        iter->entries_count+=1;
		return record;
    }

    return NULL;
}

xm_shm_entry_iterator_t * xm_shm_entry_iterator_prefare(xm_shm_format_t *fmt){

    xm_shm_entry_t *shm_entry = &fmt->cur_header.shm_entry;
	xm_shm_entry_iterator_t *g_iter_ptr = &fmt->shm_iter;

    g_iter_ptr->fmt = fmt;
    g_iter_ptr->header = NULL;
    g_iter_ptr->next = record_next;
    g_iter_ptr->entries_count = 0;

    if( -1 == xm_shm_entry_get(fmt->shm_int,shm_entry)){
        return NULL;
    }

    xm_bf_init(&fmt->cur_header.bfmt,shm_entry->start,shm_entry->end);

    if(-1 == _shm_entry_header_read(&fmt->cur_header,&fmt->cur_header.bfmt))
        return NULL;

    g_iter_ptr->header = &fmt->cur_header;

    return g_iter_ptr;
}

void xm_shm_entry_iterator_commit(xm_shm_format_t *fmt,xm_shm_entry_iterator_t *iter){

    if(iter->header){
        xm_shm_entry_t *shm_entry = &iter->header->shm_entry;
        xm_shm_entry_commit(fmt->shm_int,shm_entry);
        iter->header = NULL;
        iter->entries_count = 0;
    }

}
