##########################################################
#Author(A) shajianfeng
##########################################################
include ../make.include

CFLAGS  = ${BUILD_CFLAGS}  -g -rdynamic  -pthread  
CFLAGS += -I../lib/common 
CFLAGS += -I../lib/thread
CFLAGS += -I../lib/shm
CFLAGS += -I./probe
CFLAGS += -I./interface
CFLAGS += -I.

LDFLAGS  = ${BUILD_LDFLAGS} -lpthread -lpfring -lpcap -lgmp 

common_package = ../lib/common/*.o
thread_package = ../lib/thread/*.o
shm_package=../lib/shm/*.o
interface_package=interface/*.o
probe_package=probe/*.o

xmap_package = xmap

xmap_SOURCES = xmap.c \
			   xopt.c \
			   xm_wblist.c \
			   xm_aesrand.c \
			   xm_cyclic.c \
			   xm_shard.c \
			   xm_store.c \
			   xm_validate.c \
			   xm_context.c \
			   xm_send_task.c \
			   xm_recv_task.c \
			   xm_work.c

xmap_OBJECTS =$(patsubst %.c,%.o,$(xmap_SOURCES))
xmap_DEPENDS =$(patsubst %.c,%.d,$(xmap_SOURCES))
xmap_ASMFILE =$(patsubst %.c,%.s,$(xmap_SOURCES))

quiet_cmd_xmap = LINK   $@ 
	cmd_xmap = ${CC} ${CFLAGS} -o $@ $(xmap_OBJECTS)  \
				 $(common_package)  $(thread_package) \
				 $(shm_package) $(interface_package) $(probe_package) \
				   $(LDFLAGS)

.PHONY: all clean

all: $(xmap_package)

$(xmap_package): $(xmap_OBJECTS) 
	$(call cmd,xmap)
-include $(xmap_DEPENDS)

$(common_package):
	$(call,make)

$(thread_package):
	$(call,make)

$(shm_package):
	$(call,make)

$(probe_package):
	$(call,make)

$(interface_package):
	$(call,make)

clean:
	@rm -rf $(xmap_OBJECTS) $(xmap_DEPENDS) $(xmap_ASMFILE) $(xmap_package)

