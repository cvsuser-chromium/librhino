# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
    'targets': [
    {
        'target_name': 'libupnp',
            'type': 'static_library',
            'dependencies': [
            ],
            'defines': [
              'HAVE_CONFIG_H',
            ],
            'defines!': [
              #libupnp don't need unicode support.
              'UNICODE',
            ],
            'include_dirs': [
              'build/inc',
              'threadutil/inc',
              'ixml/inc',
              'ixml/src/inc',
              'upnp/inc',
              'upnp/src/inc',
            ],
            'direct_dependent_settings': {
                'defines': [
                ],
                'include_dirs': [
                  '.'
                  'build/inc',
                  'upnp/inc',
                  'ixml/inc',
                ],
                'linkflags': [

                ],
            },
            'export_dependent_settings': [
            ],
            'sources': [
               './threadutil/src/FreeList.c',
               './threadutil/src/LinkedList.c',
               './threadutil/src/ThreadPool.c',
               './threadutil/src/TimerThread.c',
               './ixml/src/attr.c',
               './ixml/src/document.c',
               './ixml/src/element.c',
               './ixml/src/ixml.c',
               './ixml/src/ixmldebug.c',
               './ixml/src/ixmlmembuf.c',
               './ixml/src/ixmlparser.c',
               './ixml/src/namedNodeMap.c',
               './ixml/src/node.c',
               './ixml/src/nodeList.c',
               './upnp/src/api/upnpapi.c',
               './upnp/src/api/upnpdebug.c',
               './upnp/src/api/UpnpString.c',
               './upnp/src/api/upnptools.c',
               './upnp/src/gena/gena_callback2.c',
               './upnp/src/gena/gena_ctrlpt.c',
               './upnp/src/gena/gena_device.c',
               './upnp/src/genlib/client_table/client_table.c',
               './upnp/src/genlib/miniserver/miniserver.c',
               './upnp/src/genlib/net/http/httpparser.c',
               './upnp/src/genlib/net/http/httpreadwrite.c',
               './upnp/src/genlib/net/http/parsetools.c',
               './upnp/src/genlib/net/http/statcodes.c',
               './upnp/src/genlib/net/http/webserver.c',
               './upnp/src/genlib/net/sock.c',
               './upnp/src/genlib/net/uri/uri.c',
               './upnp/src/genlib/service_table/service_table.c',
               './upnp/src/genlib/util/membuffer.c',
               './upnp/src/genlib/util/strintmap.c',
               './upnp/src/genlib/util/upnp_timeout.c',
               './upnp/src/genlib/util/util.c',  
               './upnp/src/soap/soap_common.c',
               './upnp/src/soap/soap_ctrlpt.c',
               './upnp/src/soap/soap_device.c',
               './upnp/src/ssdp/ssdp_ctrlpt.c',
               './upnp/src/ssdp/ssdp_device.c',
               './upnp/src/ssdp/ssdp_server.c',
               './upnp/src/urlconfig/urlconfig.c',
               './upnp/src/uuid/md5.c',
               './upnp/src/uuid/sysdep.c',
               './upnp/src/uuid/uuid.c',
            ],
            'conditions': [
              ['OS=="linux"', {
                  'defines': [
                      'LINUX_DEFINE',
                  ],
                  'include_dirs': [
                      'include/linux',
                  ],
                  'cflags': [
                    '-g',
                    '-O2',
                    '-Wall',
                  ]
              }],
                ['OS=="win"', {
                  'defines': [
                    'WIN32',
                    'UPNP_STATIC_LIB',
                    '__PRETTY_FUNCTION__=__FUNCTION__',
                    '_USRDLL',
                    'UPNP_USE_MSVCPP',
                    '_CRT_NONSTDC_NO_WARNINGS',
                    '_CRT_NONSTDC_NO_DEPRECATE',
                    '_CRT_SECURE_NO_WARNINGS',
                    '_CRT_SECURE_NO_DEPRECATE',
                    '_SECURE_SCL',
                    '_SCL_SECURE_NO_WARNINGS',
                    '_SCL_SECURE_NO_DEPRECATE',
                    '_AFX_SECURE_NO_WARNINGS',
                    '_AFX_SECURE_NO_DEPRECATE',
                    '_SECURE_ATL',
                    '_ATL_NO_COM_SUPPORT',
                    '_ATL_SECURE_NO_WARNINGS',
                    '_ATL_SECURE_NO_DEPRECATE',
                  ],
                  'include_dirs': [
                    'build/msvc',                  
                    '$(WEBKITOUTPUTDIR)/include',
                    '$(WEBKITOUTPUTDIR)/include/pthread'
                  ],
                'cflags': [
                  '/W3',
                  '/wd4996',
                  '/nologo',
                ]
            }, { # OS != "win",
                'defines': [
                    'NON_WINDOWS_DEFINE',
                ],
            }]
        ]
    },
        ]
}
