
#dependent path variables, please NOT modify or change!!!!
set(DLNA_MODULES_PATH       ${CMAKE_SOURCE_DIR})
set(DLNA_DLNA_MOD_PATH      ${DLNA_MODULES_PATH}/dlna)
set(DLNA_DTCPIP_MOD_PATH    ${DLNA_MODULES_PATH}/dtcpip)
set(DLNA_JSON_MOD_PATH      ${DLNA_MODULES_PATH}/json)
set(DLNA_FFMPEG_MOD_PATH    ${DLNA_MODULES_PATH}/ffmpeg)
set(DLNA_LIBUPNP_MOD_PATH   ${DLNA_MODULES_PATH}/libupnp)

set(DLNA_SDK_C_DEVELOPER_PATH    ${DLNA_MODULES_PATH}/../SDK/for_C_developer)
set(DLNA_SDK_SHARED_LIBS_PATH    ${DLNA_SDK_C_DEVELOPER_PATH}/shared)
set(DLNA_SDK_STATIC_LIBS_PATH    ${DLNA_SDK_C_DEVELOPER_PATH}/static)
set(DLNA_SDK_INCLUDE_PATH        ${DLNA_SDK_C_DEVELOPER_PATH}/porting/include)

#detect enviroent and set related var
#set(TARGET_OS_TYPE "linux")
message(" ")
message("!!!!! NOTE: compiler prefix = $ENV{DLNA_CMAKE_PREFIX}")
message("!!!!! NOTE: build type = $ENV{DLNA_CMAKE_BUILD_TYPE}")
message(" ")

set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS ON)
option(ENABLE_DLNA_OVERSEA "ENABLE DLNA_OVERSEA" OFF)

if(EXISTS ${CMAKE_SOURCE_DIR}/TestConfig.cmake)
	message("TestConfig.cmake included")
	include(TestConfig.cmake)

else()
	message("TestConfig.cmake not exsit")
	set(CMAKE_C_COMPILER   "$ENV{DLNA_CMAKE_PREFIX}gcc")
	set(CMAKE_CXX_COMPILER "$ENV{DLNA_CMAKE_PREFIX}g++")

	if($ENV{DLNA_CMAKE_BUILD_TYPE} STREQUAL "Debug")
		set(CMAKE_BUILD_TYPE   "Debug")
		set(CXXFLAGS "-DDLNA_ENABLE_LOG -g -O3")

	elseif($ENV{DLNA_CMAKE_BUILD_TYPE} STREQUAL "debug")
		set(CMAKE_BUILD_TYPE   "Debug")
		set(CXXFLAGS "-DDLNA_ENABLE_LOG -g -O3")

	elseif($ENV{DLNA_CMAKE_BUILD_TYPE} STREQUAL "Release")
		set(CMAKE_BUILD_TYPE   "Release")
		set(CXXFLAGS "-O3")

	elseif($ENV{DLNA_CMAKE_BUILD_TYPE} STREQUAL "release")
		set(CMAKE_BUILD_TYPE   "Release")
		set(CXXFLAGS "-O3")

	else()
		message(" ")
		message("!!!!! ERROR: NOT find cmake bulid type")
		message(" ")

	endif()
endif()


set(G_BUILD_TYPE   ${CMAKE_BUILD_TYPE})



