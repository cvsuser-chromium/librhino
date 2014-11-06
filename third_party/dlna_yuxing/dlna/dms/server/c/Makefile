
#DLNA_PROJECT_PATH = $(shell pwd)/../../../..
DLNA_PROJECT_PATH = ../../../..
COMMON_PATH = $(DLNA_PROJECT_PATH)/modules/common.path
COMMON_MAK = $(DLNA_PROJECT_PATH)/modules/common.mak

ifeq (,$(wildcard $(COMMON_PATH)))
$(error "config.path is not present!!!!")
endif
include $(COMMON_PATH)

PROG = dms

SRCS =  cds.c \
        cms.c \
        msr.c \
        http.c \
        presentation.c \
        metadata.c \
        mime.c \
        services.c \
        buffer.c \
        util_iconv.c \
        content.c \
        cfgparser.c \
        trace.c \
        redblack.c \
        ushare.c
		
#       osdep.c \
#       ctrl_telnet.c \

CFLAGS	:= -W -Wall -O3 -fPIC -I../../dlna_info/src
DEFINES := -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -D_REENTRANT -D_GNU_SOURCE -DHAVE_LOCALE_H -DHAVE_SETLOCALE -DHAVE_LANGINFO_H -DHAVE_LANGINFO_CODESET -DHAVE_ICONV -DHAVE_DLNA

#LDFLAGS := -L$(ROOTFS)/home/yx5911/lib -L$(ROOTFS)/lib -L$(ROOTFS)/usr/lib -lzebra -lsettop -lnexus -lupnp -lthreadutil -lixml -ldtcpip -lrender
LDFLAGS := -L$(ROOTFS)/home/yx5911/lib -L$(ROOTFS)/lib -L$(ROOTFS)/usr/lib -lupnp -lthreadutil -lixml

all: app share
will: app
clean: clean-all

include $(COMMON_MAK)
