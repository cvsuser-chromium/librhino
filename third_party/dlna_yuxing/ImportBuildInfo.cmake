FIND_PACKAGE(Subversion)
IF(Subversion_FOUND)
	set( BUILD_INFO_HEADER
		${CMAKE_SOURCE_DIR}/dlna/dmc/src/c/build_info.h )
	Subversion_WC_INFO(${CMAKE_SOURCE_DIR} Project)
	execute_process(COMMAND date
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} 
		OUTPUT_VARIABLE CUR_TIME
		)
	execute_process(COMMAND whoami
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} 
		OUTPUT_VARIABLE SYSTEM_USERNAME
		)
	execute_process(COMMAND uname -n
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		OUTPUT_VARIABLE NODE_NAME
		)
	EXEC_PROGRAM("echo"
		ARGS "${CUR_TIME}"
		OUTPUT_VARIABLE COMPILER_CUR_TIME
		)
	EXEC_PROGRAM("echo"
		ARGS "${SYSTEM_USERNAME}"
		OUTPUT_VARIABLE COMPILER_USER_NAME
		)
	EXEC_PROGRAM("echo"
		ARGS "${NODE_NAME}"
		OUTPUT_VARIABLE COMPILER_NODE_NAME
		)
	  
#	FILE(WRITE ${BUILD_INFO_HEADER} "const char *g_make_build_date = \"${COMPILER_CUR_TIME}\";\n")
	FILE(WRITE ${BUILD_INFO_HEADER} "\#ifndef __BUILD_INFO_H__\n")
	FILE(APPEND ${BUILD_INFO_HEADER} "\#define __BUILD_INFO_H__\n\n")
	FILE(APPEND ${BUILD_INFO_HEADER} "const static char *build_date = \"${COMPILER_CUR_TIME}\";\n")
#	FILE(APPEND ${BUILD_INFO_HEADER} "const char *g_make_build_version = \"${COMPILER_CUR_TIME}\";\n")
	FILE(APPEND ${BUILD_INFO_HEADER} "const static char *build_version = \"${Project_WC_REVISION}\";\n")
	FILE(APPEND ${BUILD_INFO_HEADER} "const static char *build_type=\"$ENV{DLNA_CMAKE_BUILD_TYPE}\";\n")
	FILE(APPEND ${BUILD_INFO_HEADER} "const static char *build_platform=\"$ENV{PLATFORM}\";\n")
	FILE(APPEND ${BUILD_INFO_HEADER} "\#endif\n")
	MESSAGE("Current revision is ${Project_WC_REVISION}, build_info=${BUILD_INFO_HEADER}")
ENDIF(Subversion_FOUND)



