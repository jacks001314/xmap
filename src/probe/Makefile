##########################################################
#Copyright(C) 2012 WAF PROJECT TEAM
#Author(A) shajianfeng
##########################################################

include ../../make.include
CFLAGS  = ${BUILD_CFLAGS}  -O0 -rdynamic -D_GNU_SOURCE
CFLAGS += -I../../lib/common -I../../lib/shm -I../ -I../interface -I. 
CFLAGS +=  -fPIC

xm_probe_SOURCES = xm_packet.c \
				   xm_probe_tcpsynscan.c \
				   xm_probe_tcpsynackscan.c \
				   xm_probe_icmp.c \
				   xm_probe_icmp_time.c \
				   xm_probe.c

xm_probe_OBJECTS = $(patsubst %.c,%.o,$(xm_probe_SOURCES))
xm_probe_DEPENDS = $(patsubst %.c,%.d,$(xm_probe_SOURCES))
xm_probe_ASMFILE = $(patsubst %.c,%.s,$(xm_probe_SOURCES))

.PHONY: all clean

all: $(xm_probe_OBJECTS)

clean:
	@rm -fr $(xm_probe_OBJECTS) $(xm_probe_DEPENDS) $(xm_probe_ASMFILE) $(xm_probe_package)
	@rm -fr *.d *.o *.s 

