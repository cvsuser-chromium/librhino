# [Name]: Makefile
# [type]: Compile tool
# [Description]: for module AVTransport
# [version]: 0.1
# [First Written]: By Chenwei {Corp. Hybroad} #2011/1/27#
# [Write what you want]:
# 
TOP = ..
DLNA_PROJECT_PATH = $(TOP)/../../../..
include $(DLNA_PROJECT_PATH)/modules/common.path

OBJECTS := 	upnp_render.o	\
		   	upnp_util.o	

INCLUDES := -I. -I../underneath -I../jse \
			-I$(INCLUDES_PATH) \
			-I$(INCLUDES_PATH)/upnp
		
#DTCP_INC = -I../../include/dtcp/inc -I../../include/support/inc -I../../include/util/inc
#UPNP_INC = -I. -I../../include/upnp/inc -I../../include/threadutil/inc -I../../include/ixml/inc -I../../include/libzebra

#INCLUDES := -I../../include -I../HySDK -I../AVTransport
#INCLUDES += $(UPNP_INC) $(DTCP_INC)

#CFLAGS :=  -D_XOPEN_SOURCE=500 -DBROWSER_MERGE
CFLAGS :=  -D_XOPEN_SOURCE=500

all: lib

clean: clean-current


include $(TOP)/common.mak

