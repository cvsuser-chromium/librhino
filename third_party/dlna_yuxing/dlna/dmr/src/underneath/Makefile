# [Name]: Makefile
# [type]: Compile tool
# [Description]: for module AVTransport
# [version]: 0.1
# [First Written]: By Chenwei {Corp. Hybroad} #2011/1/27#
# [Write what you want]:
# 
#TOP = $(shell pwd)/..
TOP = ..
DLNA_PROJECT_PATH = $(TOP)/../../../..
include $(DLNA_PROJECT_PATH)/modules/common.path

OBJECTS := 	x_avplayer.o \
			x_imageplayer.o \
			x_renderingcontrol.o \
			dtcp_client.o 
			

INCLUDES := -I. -I../renderer -I../jse \
			-I$(INCLUDES_PATH) \
			-I$(MOD_DTCPIP_PATH)/Dtcp/inc \
			-I$(MOD_DTCPIP_PATH)/Support/inc \
			-I$(MOD_DTCPIP_PATH)/Util/inc 
			

CFLAGS := 
all: lib
clean: clean-current
include $(TOP)/common.mak
