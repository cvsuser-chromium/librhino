##################################################
#                                                #
# Main CMakeLists for libupnp Project.			     #
#                                                #
##################################################
cmake_minimum_required (VERSION 2.6)
project(libupnp CXX C)

#include (build/cmake/CheckPlatformSDK.cmake)

set (libupnp_SRC_PATH ${PROJECT_SOURCE_DIR})

list ( APPEND libupnp_SRC_C
  ${libupnp_SRC_PATH}/threadutil/src/FreeList.c
  ${libupnp_SRC_PATH}/threadutil/src/LinkedList.c
  ${libupnp_SRC_PATH}/threadutil/src/ThreadPool.c
  ${libupnp_SRC_PATH}/threadutil/src/TimerThread.c
  ${libupnp_SRC_PATH}/ixml/src/attr.c
  ${libupnp_SRC_PATH}/ixml/src/document.c
  ${libupnp_SRC_PATH}/ixml/src/element.c
  ${libupnp_SRC_PATH}/ixml/src/ixml.c
  ${libupnp_SRC_PATH}/ixml/src/ixmldebug.c
  ${libupnp_SRC_PATH}/ixml/src/ixmlmembuf.c
  ${libupnp_SRC_PATH}/ixml/src/ixmlparser.c
  ${libupnp_SRC_PATH}/ixml/src/namedNodeMap.c
  ${libupnp_SRC_PATH}/ixml/src/node.c
  ${libupnp_SRC_PATH}/ixml/src/nodeList.c
  ${libupnp_SRC_PATH}/upnp/src/api/upnpapi.c
  ${libupnp_SRC_PATH}/upnp/src/api/upnpdebug.c
  ${libupnp_SRC_PATH}/upnp/src/api/UpnpString.c
  ${libupnp_SRC_PATH}/upnp/src/api/upnptools.c
  ${libupnp_SRC_PATH}/upnp/src/gena/gena_callback2.c
  ${libupnp_SRC_PATH}/upnp/src/gena/gena_ctrlpt.c
  ${libupnp_SRC_PATH}/upnp/src/gena/gena_device.c
  ${libupnp_SRC_PATH}/upnp/src/genlib/client_table/client_table.c
  ${libupnp_SRC_PATH}/upnp/src/genlib/miniserver/miniserver.c
  ${libupnp_SRC_PATH}/upnp/src/genlib/net/http/httpparser.c
  ${libupnp_SRC_PATH}/upnp/src/genlib/net/http/httpreadwrite.c
  ${libupnp_SRC_PATH}/upnp/src/genlib/net/http/parsetools.c
  ${libupnp_SRC_PATH}/upnp/src/genlib/net/http/statcodes.c
  ${libupnp_SRC_PATH}/upnp/src/genlib/net/http/webserver.c
  ${libupnp_SRC_PATH}/upnp/src/genlib/net/sock.c
  ${libupnp_SRC_PATH}/upnp/src/genlib/net/uri/uri.c
  ${libupnp_SRC_PATH}/upnp/src/genlib/service_table/service_table.c
  ${libupnp_SRC_PATH}/upnp/src/genlib/util/membuffer.c
  ${libupnp_SRC_PATH}/upnp/src/genlib/util/strintmap.c
  ${libupnp_SRC_PATH}/upnp/src/genlib/util/upnp_timeout.c
  ${libupnp_SRC_PATH}/upnp/src/genlib/util/util.c  
  ${libupnp_SRC_PATH}/upnp/src/soap/soap_common.c
  ${libupnp_SRC_PATH}/upnp/src/soap/soap_ctrlpt.c
  ${libupnp_SRC_PATH}/upnp/src/soap/soap_device.c
  ${libupnp_SRC_PATH}/upnp/src/ssdp/ssdp_ctrlpt.c
  ${libupnp_SRC_PATH}/upnp/src/ssdp/ssdp_device.c
  ${libupnp_SRC_PATH}/upnp/src/ssdp/ssdp_server.c
  ${libupnp_SRC_PATH}/upnp/src/urlconfig/urlconfig.c
  ${libupnp_SRC_PATH}/upnp/src/uuid/md5.c
  ${libupnp_SRC_PATH}/upnp/src/uuid/sysdep.c
  ${libupnp_SRC_PATH}/upnp/src/uuid/uuid.c
  )

add_definitions(-DHAVE_CONFIG_H)

include_directories (
  $ENV{WEBKITOUTPUTDIR}/include
  build/inc
  threadutil/inc
  ixml/inc
  ixml/src/inc
  upnp/inc
  upnp/src/inc
  )

if (WIN32)
  include_directories (
    build/msvc
    )
  list ( APPEND libupnp_SRC_C
    #${libupnp_SRC_PATH}/upnp/src/win_dll.c
    #${libupnp_SRC_PATH}/upnp/src/inet_pton.c
    )
else (WIN32)
  include_directories (
    )
  list ( APPEND libupnp_SRC_C
    )
endif (WIN32)

if (1)
  add_library (libupnp STATIC ${libupnp_SRC_C})
  add_definitions( -DUPNP_STATIC_LIB )
else (1)
  add_library (libupnp SHARED ${libupnp_SRC_C})
  add_definitions( 
    -DLIBUPNP_EXPORTS 
    )

endif (1)
if (WIN32)
  add_definitions (
    -DWIN32
    -D__PRETTY_FUNCTION__=__FUNCTION__
    #-Dstrcasecmp=_stricmp
    -DDEBUG
    -D_USRDLL
    -DUPNP_USE_MSVCPP
    -D_CRT_NONSTDC_NO_WARNINGS
    -D_CRT_NONSTDC_NO_DEPRECATE
    -D_CRT_SECURE_NO_WARNINGS
    -D_CRT_SECURE_NO_DEPRECATE
    -D_SECURE_SCL
    -D_SCL_SECURE_NO_WARNINGS
    -D_SCL_SECURE_NO_DEPRECATE
    -D_AFX_SECURE_NO_WARNINGS
    -D_AFX_SECURE_NO_DEPRECATE
    -D_SECURE_ATL
    -D_ATL_NO_COM_SUPPORT
    -D_ATL_SECURE_NO_WARNINGS
    -D_ATL_SECURE_NO_DEPRECATE
    )
  set_source_files_properties(${libupnp_SRC_C}
    #PROPERTIES COMPILE_FLAGS "/W3 /wd4996 /nologo"
    )

ELSE (WIN32)
  add_definitions (
    -DOS_LINUX
    )
  set_source_files_properties(${libupnp_SRC_C}
    PROPERTIES COMPILE_FLAGS "-g -O2 -Wall"
    )
ENDIF (WIN32)


