##########################################################
#Copyright(C) 2012 WAF PROJECT TEAM
#Author(A) shajianfeng
##########################################################

CFLAGS  = ${BUILD_CFLAGS} -ggdb3 -O0 -rdynamic -D_GNU_SOURCE -pthread
CFLAGS += -fPIC -shared -I../../../lib/common -I../../../lib/shm  -I${JAVA_HOME}/include -I${JAVA_HOME}/include/linux

jni_SOURCES = ../../../lib/common/xm_file_open.c \
		  ../../../lib/common/xm_errno.c  \
		  ../../../lib/common/xm_string.c \
		  ../../../lib/common/xm_mpool.c \
		  ../../../lib/common/xm_log.c    \
		  ../../../lib/common/xm_util.c \
		  ../../../lib/shm/xm_shm_mmap.c \
		  ../../../lib/shm/xm_shm_memory.c \
		  ../../../lib/shm/xm_shm_interface.c \
		  ../../../lib/shm/xm_shm_format.c \
		  ../../../lib/shm/xm_shm_entry_record.c \
		  xm_entry_reader.c

jni_package = libXMapJNI.so

.PHONY: all clean install

all:
	gcc ${CFLAGS} ${jni_SOURCES} -o ${jni_package} ${LDLIBS} 

clean:
	rm -rf ${jni_package}
