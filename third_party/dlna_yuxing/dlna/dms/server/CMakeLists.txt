#include all .c files, you can use list or aux_source_directory
list(APPEND C_SRC_LIST
	c/cds.c
	c/cms.c
#	c/msr.c
	c/http.c
#	c/presentation.c
	c/metadata.c
	c/mime.c
	c/services.c
	c/buffer.c
#	c/util_iconv.c
	c/content.c
#	c/cfgparser.c
#	c/trace.c
	c/redblack.c
	c/add_usb.c
	c/adding_thread.c
	c/entry_json.c
	c/upnp_entry.c
	c/filetree.c
	c/ushare.c)
#aux_source_directory(c C_SRC_LIST)
#you can redefine src list here again to replace the previous 
set(MODULE_SRC_LIST ${C_SRC_LIST})


#include all .h files
include_directories(c
					${DLNA_JSON_MOD_PATH}/src/h
					${DLNA_LIBUPNP_MOD_PATH}/ixml/inc
					${DLNA_LIBUPNP_MOD_PATH}/threadutil/inc
					${DLNA_LIBUPNP_MOD_PATH}/upnp/inc
					${DLNA_SDK_INCLUDE_PATH}/platform
					../dlnaInfo/src
					${DLNA_DLNA_MOD_PATH}/common)


#add definitions
add_definitions(-D_LARGEFILE_SOURCE
				-D_REENTRANT
				-D_FILE_OFFSET_BITS=64
				-D_GNU_SOURCE
				-DHAVE_LOCALE_H
				-DHAVE_SETLOCALE
				-DHAVE_LANGINFO_H
				-DHAVE_LANGINFO_CODESET
				-DHAVE_ICONV
				-D__STRICT_ANSI__
				-DHAVE_DLNA)


#add cxx flags
#SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
#set(CMAKE_C_FLAGS "-g -W -Wall -O3 -fPIC")
set(CMAKE_C_FLAGS "${CXXFLAGS} -W -Wall -fPIC")

set(MODULE_NAME dms)
#indicate object to output
add_library(${MODULE_NAME} SHARED ${C_SRC_LIST})


#copy target and .h files to SDK and base project
include(${CMAKE_SOURCE_DIR}/PostCompile.cmake)
include(${CMAKE_SOURCE_DIR}/ImportBuildInfo.cmake)
