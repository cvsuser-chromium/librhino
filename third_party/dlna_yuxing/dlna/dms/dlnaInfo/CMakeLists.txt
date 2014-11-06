#include all .c files, you can use list or aux_source_directory
#list(APPEND C_SRC_LIST
#	ushare.c)
aux_source_directory(src C_SRC_LIST)
#you can redefine src list here again to replace the previous 
set(MODULE_SRC_LIST ${C_SRC_LIST})


#include all .h files
include_directories(src
					${DLNA_FFMPEG_MOD_PATH}
					${DLNA_FFMPEG_MOD_PATH}/libavcodec
					${DLNA_FFMPEG_MOD_PATH}/libavutil
					${DLNA_FFMPEG_MOD_PATH}/libavformat
					${DLNA_JSON_MOD_PATH}/src/h
					${DLNA_DLNA_MOD_PATH}/common)


#add definitions
add_definitions(-D_LARGEFILE_SOURCE
				-D_REENTRANT
				-D_FILE_OFFSET_BITS=64)


#add cxx flags
#SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
#set(CMAKE_C_FLAGS "-g -Wall -O3 -Os -fPIC")
set(CMAKE_C_FLAGS "${CXXFLAGS} -Wall -Os -fPIC")

set(MODULE_NAME dlnaInfo)
#indicate object to output
add_library(${MODULE_NAME} SHARED ${C_SRC_LIST})


#copy target and .h files to SDK and base project
include(${CMAKE_SOURCE_DIR}/PostCompile.cmake)
