#include all .c files, you can use list or aux_source_directory
#list( APPEND JSON_SRC_LIST
#	sys_key_deal.c
#	)
aux_source_directory(c C_SRC_LIST)
#you can redefine src list here again to replace the previous 
set(MODULE_SRC_LIST ${C_SRC_LIST})


#include all .h files
include_directories(c h
					${DLNA_JSON_MOD_PATH}/src/h
#					${DLNA_JSON_MOD_PATH}/src/c
					${DLNA_LIBUPNP_MOD_PATH}/ixml/inc
					${DLNA_LIBUPNP_MOD_PATH}/threadutil/inc
					${DLNA_LIBUPNP_MOD_PATH}/upnp/inc
					${DLNA_SDK_INCLUDE_PATH}/platform
					${DLNA_DLNA_MOD_PATH}/common)


#add definitions
add_definitions(-DHAVE_CONFIG_H 
				-D_FILE_OFFSET_BITS=64
				-D_XOPEN_SOURCE=500)


#add cxx flags
#SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
#set(CMAKE_C_FLAGS "-g -O2 -Os -Wall -pthread -fPIC")
set(CMAKE_C_FLAGS "${CXXFLAGS} -Os -Wall -pthread -fPIC")

set(MODULE_NAME dmc)
#indicate object to output
add_library(${MODULE_NAME} SHARED ${C_SRC_LIST})


#copy target and .h files to SDK and base project
include(${CMAKE_SOURCE_DIR}/PostCompile.cmake)
