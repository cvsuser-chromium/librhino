#copy target to SDK and base project
set(COPY_NEWER cp -f)

message(" ")
#message("SDK lib path = ${DLNA_SDK_SHARED_LIBS_PATH}/${G_BUILD_TYPE}")
message(" ")

# copy .so or .a
add_custom_command(TARGET ${MODULE_NAME} POST_BUILD
#	COMMAND echo "${COPY_NEWER} lib${MODULE_NAME}.so to ${DLNA_SDK_SHARED_LIBS_PATH}"
	COMMAND ${COPY_NEWER}  lib${MODULE_NAME}.so ${DLNA_SDK_SHARED_LIBS_PATH}/${G_BUILD_TYPE}
	COMMENT "${COPY_NEWER} lib${MODULE_NAME}.so to SDK in ${DLNA_SDK_SHARED_LIBS_PATH}/${G_BUILD_TYPE}"
    VERBATIM 
) 

#copy .h files
#1. dlna related
if(${MODULE_NAME} STREQUAL "dmc")
	set(HEAD_FILES dlna_api.h  dlna_audio_type.h  dlna_type.h  dlna_type_private.h  hitTime.h coo.h dms.h)
	add_custom_command(TARGET ${MODULE_NAME} POST_BUILD
		COMMAND ${COPY_NEWER}  ${HEAD_FILES} ${DLNA_SDK_INCLUDE_PATH}/dlna/
		WORKING_DIRECTORY ${DLNA_DLNA_MOD_PATH}/common
		COMMENT "${COPY_NEWER} ${HEAD_FILES} to SDK"
		VERBATIM 
	) 	
endif()

#2. upnp related
if(${MODULE_NAME} STREQUAL "ixml")
	set(HEAD_FILES ixml.h)
	add_custom_command(TARGET ${MODULE_NAME} POST_BUILD
		COMMAND ${COPY_NEWER}  ${HEAD_FILES} ${DLNA_SDK_INCLUDE_PATH}/upnp/
		WORKING_DIRECTORY ${DLNA_LIBUPNP_MOD_PATH}/ixml/inc
		COMMENT "${COPY_NEWER} ${HEAD_FILES} to SDK"
		VERBATIM 
	) 
endif()
if(${MODULE_NAME} STREQUAL "threadutil")
	set(HEAD_FILES FreeList.h  ithread.h  LinkedList.h)
	add_custom_command(TARGET ${MODULE_NAME} POST_BUILD
		COMMAND ${COPY_NEWER}  ${HEAD_FILES} ${DLNA_SDK_INCLUDE_PATH}/upnp/
		WORKING_DIRECTORY ${DLNA_LIBUPNP_MOD_PATH}/threadutil/inc
		COMMENT "${COPY_NEWER} ${HEAD_FILES} to SDK"
		VERBATIM 
	) 
endif()
if(${MODULE_NAME} STREQUAL "upnp")
	set(HEAD_FILES upnpconfig.h  upnp.h  upnptools.h)
	add_custom_command(TARGET ${MODULE_NAME} POST_BUILD
		COMMAND ${COPY_NEWER}  ${HEAD_FILES} ${DLNA_SDK_INCLUDE_PATH}/upnp/
		WORKING_DIRECTORY ${DLNA_LIBUPNP_MOD_PATH}/upnp/inc
		COMMENT "${COPY_NEWER} ${HEAD_FILES} to SDK"
		VERBATIM 
	) 
endif()

