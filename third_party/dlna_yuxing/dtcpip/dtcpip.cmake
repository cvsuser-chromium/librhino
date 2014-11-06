#include all .c files, you can use list or aux_source_directory
#list( APPEND JSON_SRC_LIST
#	sys_key_deal.c
#	)
aux_source_directory(Dtcp/src/Aes C_SRC_LIST)
aux_source_directory(Dtcp/src/DtcpApi C_SRC_LIST)
aux_source_directory(Dtcp/src/DtcpCore C_SRC_LIST)
aux_source_directory(Dtcp/src/DtcpIpTransport C_SRC_LIST)
aux_source_directory(Dtcp/src/DtcpAkeCore C_SRC_LIST)
aux_source_directory(Dtcp/src/DtcpCert C_SRC_LIST)

aux_source_directory(Dtcp/src/DtcpEcCrypto C_SRC_LIST)
aux_source_directory(Dtcp/src/DtcpEcCrypto/icl/src/ICLProc C_SRC_LIST)
aux_source_directory(Dtcp/src/DtcpEcCrypto/icl/src/rsa1024 C_SRC_LIST)
aux_source_directory(Dtcp/src/DtcpEcCrypto/icl/src/rsakg C_SRC_LIST)
aux_source_directory(Dtcp/src/DtcpEcCrypto/icl/src/sha C_SRC_LIST)

aux_source_directory(Dtcp/src/DtcpSrm C_SRC_LIST)
aux_source_directory(Dtcp/src/DtcpAkeIp C_SRC_LIST)
aux_source_directory(Dtcp/src/DtcpContentManagement C_SRC_LIST)
#aux_source_directory(Dtcp/src/DtcpIPPEccWrapper C_SRC_LIST)
aux_source_directory(Dtcp/src/Rng C_SRC_LIST)

aux_source_directory(Support/src/DtcpAppLib C_SRC_LIST)
aux_source_directory(Support/src/DtcpPacketizer C_SRC_LIST)

aux_source_directory(Util/src/BasicP2P C_SRC_LIST)
aux_source_directory(Util/src/InetWrapper C_SRC_LIST)
aux_source_directory(Util/src/OsWrapper C_SRC_LIST)
#you can redefine src list here again to replace the previous 
set(MODULE_SRC_LIST ${C_SRC_LIST})


#include all .h files
include_directories(Dtcp/inc
					Dtcp/src/DtcpEcCrypto
					Dtcp/src/DtcpEcCrypto/icl/include
					Support/inc
#					${DLNA_DLNA_MOD_PATH}/common
					Util/inc)


#add definitions
add_definitions(-DLINUX_BUILD 
#				-D__cdecl=""
#				-D_cdecl=""
#				-D__stdcall=""
#				-D__int64="long long"
				-D__cdecl= 
				-D_cdecl= 
				-D__stdcall= 
				-D__int64="long long" 
				-D_POSIX)

				
#add cxx flags
#SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
set(CMAKE_C_FLAGS "${CXXFLAGS} -fPIC -Wall -MP -MMD")


#set(MODULE_NAME upnp)
#indicate object to output
add_library(${MODULE_NAME} SHARED ${C_SRC_LIST})


#copy target and .h files to SDK and base project
include(${CMAKE_SOURCE_DIR}/PostCompile.cmake)