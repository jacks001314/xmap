##########################################################
#Copyright(C) 2012 WAF PROJECT TEAM
#Author(A) shajianfeng
##########################################################

include ../../make.include
CFLAGS  = ${BUILD_CFLAGS}  -O0 -rdynamic -D_GNU_SOURCE -DUSE_DPDK -pthread
CFLAGS += -I../common 
CFLAGS +=  -fPIC

xm_thread_SOURCES = xm_core_pool.c \
					xm_task.c

xm_thread_OBJECTS = $(patsubst %.c,%.o,$(xm_thread_SOURCES))
xm_thread_DEPENDS = $(patsubst %.c,%.d,$(xm_thread_SOURCES))
xm_thread_ASMFILE = $(patsubst %.c,%.s,$(xm_thread_SOURCES))

.PHONY: all clean

all: $(xm_thread_OBJECTS)

clean:
	@rm -fr $(xm_thread_OBJECTS) $(xm_thread_DEPENDS) $(xm_thread_ASMFILE) $(xm_thread_package)
	@rm -fr *.d *.o *.s 

