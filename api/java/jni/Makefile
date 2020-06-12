##########################################################
#Copyright(C) 2012 WAF PROJECT TEAM
#Author(A) shajianfeng
##########################################################

CFLAGS  = ${BUILD_CFLAGS} -ggdb3 -O0 -rdynamic -D_GNU_SOURCE
CFLAGS += -fPIC -shared -I../../../lib/common -I../../../lib/shm  -I../../../src -I${JAVA_HOME}/include -I${JAVA_HOME}/include/linux
LDFLAGS  = ${BUILD_LDFLAGS} -lgmp

jni_SOURCES = ../../../lib/common/xm_file_open.c \
		  ../../../lib/common/xm_errno.c  \
		  ../../../lib/common/xm_fileacc.c  \
		  ../../../lib/common/xm_string.c \
		  ../../../lib/common/xm_mpool.c \
		  ../../../lib/common/xm_log.c    \
		  ../../../lib/common/xm_iptree.c    \
		  ../../../lib/common/xm_xalloc.c    \
		  ../../../lib/common/xm_printf.c    \
		  ../../../lib/common/xm_time.c    \
		  ../../../lib/common/xm_strtime.c   \
		  ../../../lib/common/xm_util.c \
		  ../../../lib/common/xm_file_fullrw.c \
		  ../../../lib/common/xm_file_readwrite.c \
		  ../../../lib/common/xm_random.c \
		  ../../../lib/common/rijndael-alg-fst.c \
		  ../../../lib/shm/xm_shm_mmap.c \
		  ../../../lib/shm/xm_shm_memory.c \
		  ../../../lib/shm/xm_shm_interface.c \
		  ../../../lib/shm/xm_shm_format.c \
		  ../../../lib/shm/xm_shm_entry_record.c \
		  ../../../src/xm_aesrand.c \
		  ../../../src/xm_wblist.c \
		  ../../../src/xm_cyclic.c \
		  ../../../src/xm_shard.c \
		  ../../../src/xm_validate.c \
		  xm_entry_reader.c\
		  xm_ip_iterator.c

jni_package = libXMapJNI.so

.PHONY: all clean install

all:
	gcc ${CFLAGS} ${jni_SOURCES} -o ${jni_package} ${LDFLAGS} 

clean:
	rm -rf ${jni_package}
