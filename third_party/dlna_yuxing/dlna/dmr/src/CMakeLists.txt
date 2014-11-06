#include all .c files, you can use list or aux_source_directory
#list( APPEND JSON_SRC_LIST
#	sys_key_deal.c
#	)
aux_source_directory(jse C_SRC_LIST)
aux_source_directory(renderer C_SRC_LIST)
aux_source_directory(underneath C_SRC_LIST)
#you can redefine src list here again to replace the previous 
set(MODULE_SRC_LIST ${C_SRC_LIST})


#include all .h files
include_directories(jse renderer underneath
					${DLNA_JSON_MOD_PATH}/src/h
					
					${DLNA_LIBUPNP_MOD_PATH}/ixml/inc
					${DLNA_LIBUPNP_MOD_PATH}/threadutil/inc
					${DLNA_LIBUPNP_MOD_PATH}/upnp/inc
					
					${DLNA_DTCPIP_MOD_PATH}/Dtcp/inc
					${DLNA_DTCPIP_MOD_PATH}/Support/inc
					${DLNA_DTCPIP_MOD_PATH}/Util/inc
					
					${DLNA_SDK_INCLUDE_PATH}/platform
					
					${DLNA_DLNA_MOD_PATH}/dmc/src/h
					${DLNA_DLNA_MOD_PATH}/common)

					
#add definitions
add_definitions(-D_FILE_OFFSET_BITS=64
				-D_LARGEFILE64_SOURCE
				-D_XOPEN_SOURCE=600)


#add cxx flags
#SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
#set(CMAKE_C_FLAGS "-g -fpic -fomit-frame-pointer -Wall -Wmissing-prototypes -mips32 -pipe -O2")
set(CMAKE_C_FLAGS "${CXXFLAGS} -fpic -fomit-frame-pointer -Wall -Wmissing-prototypes -mips32 -pipe")

set(MODULE_NAME dmr)
#indicate object to output
add_library(${MODULE_NAME} SHARED ${C_SRC_LIST})


#copy target and .h files to SDK and base project
include(${CMAKE_SOURCE_DIR}/PostCompile.cmake)
