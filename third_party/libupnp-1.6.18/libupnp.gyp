# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'targets': [
  {
    'configurations': {
      'Debug': {
        'defines': [
           'DEBUG',
        ],
      },
      'Release': {
      },
    },
    'target_name': 'libupnp',
    'type': 'static_library',
    'dependencies': [
      '<(DEPTH)/base/base.gyp:base',
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
        'UPNP_STATIC_LIB',
      ],
      'include_dirs': [
        '.',
        'build/inc',
        'upnp/inc',
        'ixml/inc',
        'threadutil/inc',
      ],
    },
    'export_dependent_settings': [
    ],
    'sources': [
      './threadutil/src/FreeList.cc',
      './threadutil/src/LinkedList.cc',
      './threadutil/src/ThreadPool.cc',
      './threadutil/src/TimerThread.cc',
      './ixml/src/attr.cc',
      './ixml/src/document.cc',
      './ixml/src/element.cc',
      './ixml/src/ixml.cc',
      './ixml/src/ixmldebug.cc',
      './ixml/src/ixmlmembuf.cc',
      './ixml/src/ixmlparser.cc',
      './ixml/src/namedNodeMap.cc',
      './ixml/src/node.cc',
      './ixml/src/nodeList.cc',
      './upnp/src/api/upnpapi.cc',
      './upnp/src/api/upnpdebug.cc',
      './upnp/src/api/UpnpString.cc',
      './upnp/src/api/upnptools.cc',
      './upnp/src/gena/gena_callback2.cc',
      './upnp/src/gena/gena_ctrlpt.cc',
      './upnp/src/gena/gena_device.cc',
      './upnp/src/genlib/client_table/client_table.cc',
      './upnp/src/genlib/miniserver/miniserver.cc',
      './upnp/src/genlib/net/http/httpparser.cc',
      './upnp/src/genlib/net/http/httpreadwrite.cc',
      './upnp/src/genlib/net/http/parsetools.cc',
      './upnp/src/genlib/net/http/statcodes.cc',
      './upnp/src/genlib/net/http/webserver.cc',
      './upnp/src/genlib/net/sock.cc',
      './upnp/src/genlib/net/uri/uri.cc',
      './upnp/src/genlib/service_table/service_table.cc',
      './upnp/src/genlib/util/membuffer.cc',
      './upnp/src/genlib/util/strintmap.cc',
      './upnp/src/genlib/util/upnp_timeout.cc',
      './upnp/src/genlib/util/util.cc',  
      './upnp/src/soap/soap_common.cc',
      './upnp/src/soap/soap_ctrlpt.cc',
      './upnp/src/soap/soap_device.cc',
      './upnp/src/ssdp/ssdp_ctrlpt.cc',
      './upnp/src/ssdp/ssdp_device.cc',
      './upnp/src/ssdp/ssdp_server.cc',
      './upnp/src/urlconfig/urlconfig.cc',
      './upnp/src/uuid/md5.cc',
      './upnp/src/uuid/sysdep.cc',
      './upnp/src/uuid/uuid.cc',
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
        ],
        'link_settings': {
          'ldflags': [
            '-L$(WEBKITOUTPUTDIR)/lib',
          ],
          'library_dirs': [
            '$(WEBKITOUTPUTDIR)/lib',
          ],
          'libraries': [
            '-l$(WEBKITOUTPUTDIR)/lib/pthreadVC2',
            '-lIphlpapi',
          ],          
        },
      }]
    ]
  },
 ]
}
