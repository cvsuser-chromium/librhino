#include all .c files, you can use list or aux_source_directory
#list( APPEND JSON_SRC_LIST
#	sys_key_deal.c
#	)
set(MODULE_NAME avcodec)
set(LIB_MOD_NAME lib${MODULE_NAME})
include(${LIB_MOD_NAME}.cmake)
#include(libavutil.cmake)

#you can redefine src list here again to replace the previous 
set(MODULE_SRC_LIST ${C_SRC_LIST})


#include all .h files
include_directories(.
					..)


#add definitions
add_definitions(-D_ISOC99_SOURCE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_POSIX_C_SOURCE=200112 -D_XOPEN_SOURCE=600 -DPIC -DHAVE_AV_CONFIG_H)


#add cxx flags
#SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
#set(CMAKE_C_FLAGS "-pthread -g -O2 -Os -Wall -MD -MP")
set(CMAKE_C_FLAGS "${CXXFLAGS} -std=c99 -fomit-frame-pointer -fPIC -Wdeclaration-after-statement -Wall -Wno-parentheses -Wno-switch -Wdisabled-optimization -Wpointer-arith -Wredundant-decls -Wno-pointer-sign -Wcast-qual -Wwrite-strings -Wundef -Wmissing-prototypes -Wno-pointer-to-int-cast -fno-math-errno -fno-tree-vectorize -Werror=implicit-function-declaration -Werror=missing-prototypes")

#set(CMAKE_C_FLAGS "-g -O2 -Os -fPIC -pthread -Wall")
set(CMAKE_SHARED_LINKER_FLAGS "-Wl,-Bsymbolic -Wl,--version-script,${DLNA_FFMPEG_MOD_PATH}/${LIB_MOD_NAME}/${LIB_MOD_NAME}.ver -avoid-version -Wl,--as-needed -Wl,--warn-common")


#indicate object to output
add_library(${MODULE_NAME} SHARED ${C_SRC_LIST})


#copy target and .h files to SDK and base project
include(${CMAKE_SOURCE_DIR}/PostCompile.cmake)
