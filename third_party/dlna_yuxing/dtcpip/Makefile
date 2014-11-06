#############################################################################
## Copyright (c) 2004 Intel Corporation All Rights Reserved. 
## 
## The source code contained or described herein and all documents related to
## the source code ("Material") are owned by Intel Corporation or its 
## suppliers or licensors. Title to the Material remains with Intel 
## Corporation or its suppliers and licensors. The Material contains trade 
## secrets and proprietary and confidential information of Intel or its
## suppliers and licensors. The Material is protected by worldwide copyright
## and trade secret laws and treaty provisions. No part of the Material may 
## be used, copied, reproduced, modified, published, uploaded, posted, 
## transmitted, distributed, or disclosed in any way without Intel's prior 
## express written permission.
## 
## No license under any patent, copyright, trade secret or other 
## intellectual property right is granted to or conferred upon you by 
## disclosure or delivery of the Materials, either expressly, by 
## implication, inducement, estoppel or otherwise. Any license under such 
## intellectual property rights must be express and approved by Intel in 
## writing.
#############################################################################

#MAKEFILE FOR ALL LIBRARIES

#Targets for each library
AES = Dtcp/src/Aes/linux
AKECORE = Dtcp/src/DtcpAkeCore/linux
AKEIP = Dtcp/src/DtcpAkeIp/linux
API = Dtcp/src/DtcpApi/linux
CERT = Dtcp/src/DtcpCert/linux
CONTENTMANAGEMENT = Dtcp/src/DtcpContentManagement/linux
CORE = Dtcp/src/DtcpCore/linux
ECCRYPTO = Dtcp/src/DtcpEcCrypto/linux
IPPECCWRAPPER = Dtcp/src/DtcpIPPEccWrapper/linux
IPTRANSPORT = Dtcp/src/DtcpIpTransport/linux
SRM = Dtcp/src/DtcpSrm/linux
RNG = Dtcp/src/Rng/linux
BASICP2P = Util/src/BasicP2P/linux
INETWRAP = Util/src/InetWrapper/linux	
OSWRAP = Util/src/OsWrapper/linux
APPLIB = Support/src/DtcpAppLib/linux
PACKETIZER = Support/src/DtcpPacketizer/linux

IPPCPLIB = Ippcp/ippcp/make

DTCPCLIENT = Samples/src/DtcpHttpClient/linux
DTCPSERVER = Samples/src/Micro_Av_Server/Posix

TARGETS = Aes AkeCore AkeIp Api Cert ContentManagement Core \
	EcCrypto IpTransport Srm \
	Rng BasicP2P InetWrapper OsWrapper AppLib Packetizer 
	
#	Rng BasicP2P InetWrapper OsWrapper IppEccWrapper AppLib Packetizer 

MAKE_APP = $(MAKE)

ifeq ($(IPP),1)
	TARGETS += IppcpLib
	MAKE_APP += -f makeipp
endif

ifeq ($(SAMPLES),1)
	TARGETS += DtcpClient DtcpServer
endif

ifeq (DBG,1)
	OBJ = "DBG=1"
endif

OBJ_PATH=.libs
SLIB=libdtcpip.so
share_LIB=$(OBJ_PATH)/$(SLIB)
CC=mipsel-linux-gcc


all: alltargets

#include Makefile.include

alltargets: $(TARGETS) $(share_LIB)

$(share_LIB): 
	cd $(OBJ_PATH)
	$(CC) -shared -o $@ $(OBJ_PATH)/*.o
	cp $@ ../../libs/shared/$(SLIB)
	
Aes:
	@echo "***Building Aes Library***"
	cd $(AES) && $(MAKE)

AkeCore: 
	@echo "***Building AkeCore Library***"
	cd $(AKECORE) && $(MAKE)

AkeIp: 
	@echo "***Building AkeIp Library***"
	cd $(AKEIP) && $(MAKE)

Api:
	@echo "***Building AkeIp Library***"
	cd $(API) && $(MAKE)

Cert:
	@echo "***Building Cert Library***"
	cd $(CERT) && $(MAKE) 

ContentManagement:
	@echo "***Building Content Management Library***"
	cd $(CONTENTMANAGEMENT) && $(MAKE) 

Core:
	@echo "***Building Core Library***"
	cd $(CORE) && $(MAKE)

EcCrypto:  
	@echo "***Building EcCrypto Library***"
	cd $(ECCRYPTO) && $(MAKE)

IppEccWrapper:  
	@echo "***Building IPP Wrapper Library***"
	cd $(IPPECCWRAPPER) && $(MAKE)

IpTransport:
	@echo "***Building IpTransport Library***"
	cd $(IPTRANSPORT) && $(MAKE)

Srm:
	@echo "***Building Srm Library***"
	cd $(SRM) && $(MAKE)

Rng:
	@echo "***Building Rng Library***"
	cd $(RNG) && $(MAKE)

BasicP2P:
	@echo "***Building BasicP2P Library***"
	cd $(BASICP2P) && $(MAKE)

InetWrapper:
	@echo "***Building InetWrapper Library***"
	cd $(INETWRAP) && $(MAKE)

OsWrapper: 
	@echo "***Building OsWrapper Library***"
	cd $(OSWRAP) && $(MAKE)

AppLib:
	@echo "***Building DtcpAppLib Library***"
	cd $(APPLIB) && $(MAKE)

Packetizer:
	@echo "***Building DtcpPacketizer Library***"
	cd $(PACKETIZER) && $(MAKE)

IppcpLib:
	@echo "***Building Ippcp Library***"
	cd $(IPPCPLIB) && $(MAKE)

DtcpClient:
	@echo "***Building DtcpHttpClient Application***"
	cd $(DTCPCLIENT) && $(MAKE_APP)

DtcpServer:
	@echo "***Building Micro_Av_Server Application***"
	cd $(DTCPSERVER) && $(MAKE_APP)

clean:
	cd $(AES) && $(MAKE) clean
	cd $(AKECORE) && $(MAKE) clean
	cd $(AKEIP) && $(MAKE) clean
	cd $(API) && $(MAKE) clean
	cd $(CERT) && $(MAKE) clean
	cd $(CONTENTMANAGEMENT) && $(MAKE) clean
	cd $(CORE) && $(MAKE) clean
	cd $(ECCRYPTO) && $(MAKE) clean
	cd $(IPPECCWRAPPER) && $(MAKE) clean
	cd $(IPTRANSPORT) && $(MAKE) clean
	cd $(SRM) && $(MAKE) clean
	cd $(RNG) && $(MAKE) clean
	cd $(BASICP2P) && $(MAKE) clean
	cd $(INETWRAP) && $(MAKE) clean
	cd $(OSWRAP) && $(MAKE) clean
	cd $(APPLIB) && $(MAKE) clean
	cd $(PACKETIZER) && $(MAKE) clean
	cd $(OBJ_PATH) && rm *
	
ifeq ($(IPP), 1)
	cd $(IPPCPLIB) && $(MAKE) cleanall
endif
ifeq ($(SAMPLES),1)
	cd $(DTCPCLIENT) && $(MAKE) clean
	cd $(DTCPSERVER) && $(MAKE) clean
endif

