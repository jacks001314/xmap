##########################################################
#Copyright(C) 2012 WAF PROJECT TEAM
#Author(A) shajianfeng
##########################################################

include ../../make.include
CFLAGS  = ${BUILD_CFLAGS}  -O0 -rdynamic -D_GNU_SOURCE
CFLAGS += -I../../lib/common 
CFLAGS +=  -fPIC

xm_interface_SOURCES = xm_if_pfring.c \
					   xm_if_pfring_context.c \
					   xm_if_pcap.c \
					   xm_if_pcap_context.c \
					   xm_if_socket.c \
					   xm_if.c

xm_interface_OBJECTS = $(patsubst %.c,%.o,$(xm_interface_SOURCES))
xm_interface_DEPENDS = $(patsubst %.c,%.d,$(xm_interface_SOURCES))
xm_interface_ASMFILE = $(patsubst %.c,%.s,$(xm_interface_SOURCES))

.PHONY: all clean

all: $(xm_interface_OBJECTS)

clean:
	@rm -fr $(xm_interface_OBJECTS) $(xm_interface_DEPENDS) $(xm_interface_ASMFILE) $(xm_interface_package)
	@rm -fr *.d *.o *.s 

