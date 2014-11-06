
#DLNA_PROJECT_PATH = $(shell pwd)/../../../..
DLNA_PROJECT_PATH = ../../../..
COMMON_PATH = $(DLNA_PROJECT_PATH)/modules/common.path
COMMON_MAK = $(DLNA_PROJECT_PATH)/modules/common.mak

ifeq (,$(wildcard $(COMMON_PATH)))
$(error "config.path is not present!!!!")
endif
include $(COMMON_PATH)

PROG = dmc

#SRCS = log.c object.c service.c \
	device.c device_list.c didl_object.c \
	media_file.c file_buffer.c \
	content_dir.c vfs.c djfs.c \
	upnp_util.c string_util.c xml_util.c \
	ptr_array.c talloc_util.c cache.c \
	charset.c  js_common.c js_dmscp.c js_dmrcp.c js_huawei.c js_dms.c js_yuxing.c js_dmscp_new.c js_api.c talloc.c

SRCS =  talloc.c talloc_util.c object.c log.c \
		device.c device_list.c upnp_util.c string_util.c xml_util.c  service.c  \
		dmscp.c dmrcp.c \
		js_common.c js_eventqueue.c js_api.c js_huawei.c js_yuxing.c \
		ipc.c ipc_common.c ipc_huawei.c ipc_yuxing_fake.c ipc_yuxing_object.c \
		dlna_api.c \
		hitTime.c 
		
	
CFLAGS := -g -O2 -Os -Wall -pthread -fPIC 
DEFINES := -D_FILE_OFFSET_BITS=64 -DFUSE_USE_VERSION=22 -D_XOPEN_SOURCE=500 -DHAVE_CONFIG_H

INCLUDES := -I$(MOD_FUSE_PATH)/include -I$(MOD_JSON_PATH)/src/h -I$(MOD_UPNP_PATH)/upnp
#LDFLAGS := -L$(ROOTFS)/home/yx5911/lib -L$(ROOTFS)/lib -L$(ROOTFS)/usr/lib -lzebra -lsettop -lnexus -lupnp -lthreadutil -lixml -ldtcpip -lrender
LDFLAGS := -L$(ROOTFS)/home/yx5911/lib -L$(ROOTFS)/lib -L$(ROOTFS)/usr/lib -lupnp -lthreadutil -lixml -L$(SHARED_LIBS_PATH) -ljson

all: app share
will: app
clean: clean-all


include $(COMMON_MAK)