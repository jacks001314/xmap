##########################################################
#Copyright(C) 2012 WAF PROJECT TEAM
#Author(A) shajianfeng
##########################################################

include ../../make.include
CFLAGS += -I../common

xm_shm_SOURCES = xm_shm_interface.c \
		 xm_shm_mmap.c \
		 xm_shm_memory.c \
		 xm_shm_format.c \
		 xm_shm_entry_record.c

xm_shm_OBJECTS = $(patsubst %.c,%.o,$(xm_shm_SOURCES))
xm_shm_DEPENDS = $(patsubst %.c,%.d,$(xm_shm_SOURCES))
xm_shm_ASMFILE = $(patsubst %.c,%.s,$(xm_shm_SOURCES))

.PHONY: all clean

all: $(xm_shm_OBJECTS)

clean:
	@rm -fr $(xm_shm_OBJECTS) $(xm_shm_DEPENDS) $(xm_shm_ASMFILE) $(xm_shm_package)
	@rm -fr *.d *.o *.s 

