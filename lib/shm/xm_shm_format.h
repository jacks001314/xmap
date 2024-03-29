/*
 * =====================================================================================
 *
 *       Filename:  xm_shm_format.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  11/01/2017 02:33:23 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  shajf (jacks), csp001314@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef XM_SHM_FORMAT_H
#define XM_SHM_FORMAT_H

typedef struct xm_shm_format_t xm_shm_format_t;
typedef struct xm_shm_entry_header_t xm_shm_entry_header_t;
typedef struct xm_shm_record_t xm_shm_record_t;
typedef struct xm_shm_entry_iterator_t xm_shm_entry_iterator_t;

#include "xm_shm_interface.h"
#include "xm_bin_format.h"

#define FIMAGIC 0XFFFA
#define EIMAGIC 0XFFFB
#define MAJOR_VERSION  1
#define MINJOR_VERSION 0

/*the packet type*/
#define PACKET_TYPE_DATA    0
#define PACKET_TYPE_CLOSE   1
#define PACKET_TYPE_FLUSH 2

/*the session direct*/
#define SESSION_DIRECT_REQ 0
#define SESSION_DIRECT_RES 1

/*the protocol id*/
#define PROTOCOL_HTTP 0
#define PROTOCOL_SMTP 1
#define PROTOCOL_POP3 2
#define PROTOCOL_IMAP 3
#define PROTOCOL_DNS  4
#define PROTOCOL_DEBUG 5
#define PROTOCOL_TELNET 6
#define PROTOCOL_FTP 7
#define PROTOCOL_SMON 8
#define PROTOCOL_SSH 9
#define PROTOCOL_MAX 10

struct xm_shm_entry_header_t {

   xm_shm_entry_t shm_entry;
   xm_bin_format_t bfmt;

    uint32_t magic;
    uint32_t major_version;
    uint32_t minor_version;
    uint32_t entries_number;
    uint32_t entries_size;
};
#define XM_SHM_ENTRY_HEADER_LEN (sizeof(uint32_t)*5)

struct xm_shm_entry_iterator_t {

   xm_shm_format_t *fmt;
   xm_shm_entry_header_t *header;
   xm_shm_record_t* (*next)(xm_shm_entry_iterator_t *iter);
   uint32_t entries_count;
};

struct xm_shm_format_t {

   xm_shm_entry_header_t cur_header;
   xm_shm_entry_iterator_t shm_iter;
   xm_shm_record_t *shm_record;

   xm_pool_t *mp;
   xm_shm_interface_t *shm_int;
   int has_header;
   uint32_t entry_size;
   uint32_t record_header_size;

   void (*shm_record_write)(xm_shm_format_t *fmt,xm_shm_record_t *record);
   int  (*shm_record_read)(xm_shm_entry_iterator_t *iter,xm_bin_format_t *bfmt,xm_shm_record_t *shm_record);
};



struct xm_shm_record_t {

   uint32_t magic;
   uint32_t record_size;
   uint32_t data_offset;
   size_t data_len;
   void *data;
};

static inline uint32_t xm_shm_record_len(xm_shm_format_t *fmt,xm_shm_record_t *record){

    return record->data_len+fmt->record_header_size;
}


extern xm_shm_format_t * xm_shm_format_create(xm_pool_t *mp,xm_shm_interface_t *shm_int,uint32_t entry_size,
  uint32_t record_header_size,
  xm_shm_record_t *shm_record,
  void (*shm_record_write)(xm_shm_format_t *fmt,xm_shm_record_t *record),
  int (*shm_record_read)(xm_shm_entry_iterator_t *iter,xm_bin_format_t *bfmt,xm_shm_record_t *shm_record));


extern int xm_shm_format_put(xm_shm_format_t *fmt,xm_shm_record_t *record);

extern void xm_shm_format_flush(xm_shm_format_t *fmt);

extern xm_shm_entry_iterator_t * xm_shm_entry_iterator_prefare(xm_shm_format_t *fmt);

extern void xm_shm_entry_iterator_commit(xm_shm_format_t *fmt,xm_shm_entry_iterator_t *iter);

#endif /*XM_SHM_FORMAT_H*/

