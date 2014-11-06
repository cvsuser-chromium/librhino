#compile library
set(MODULE_NAME dtcpip)
#set(MODULE_SRC_LIST "")
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS ON)

if(PROJECT_NAME STREQUAL "BigDlna")
	message(" ")
	message("!!!!! NOTE: compile ${MODULE_NAME} in BigDlna")
	message(" ")
else()
	message(" ")
	message("!!!!! NOTE: compile ${MODULE_NAME} only")
	message(" ")
	include(../DlnaCommonConfig.cmake)
	project(${MODULE_NAME})
	cmake_minimum_required(VERSION 2.6)	
endif()


#all source files are in folder named src. just use add_subdirectory command
include(dtcpip.cmake)