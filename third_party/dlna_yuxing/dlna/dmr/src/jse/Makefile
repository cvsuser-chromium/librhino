# [Name]: Makefile
# [type]: Compile tool
# [Description]: for module AVTransport
# [version]: 0.1
# [First Written]: By Chenwei {Corp. Hybroad} #2011/1/27#
# [Write what you want]:
# 
#TOP = $(shell pwd)/../..
TOP = ..
DLNA_PROJECT_PATH = $(TOP)/../../../..
include $(DLNA_PROJECT_PATH)/modules/common.path

OBJECTS := 	direct_huawei.o \
			direct_yuxing.o \
			mrcp_yuxing.o 

INCLUDES := -I. -I../underneath -I../renderer \
			-I$(INCLUDES_PATH) \
			-I$(INCLUDES_PATH)/upnp \
			-I$(MOD_DMC_PATH)/src/c \
			-I$(MOD_JSON_PATH)/src/h 
			

CFLAGS := 

all: lib

clean: clean-current


include $(TOP)/common.mak

