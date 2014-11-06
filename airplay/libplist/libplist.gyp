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
    'target_name': 'libplist',
    'type': 'static_library',
    'dependencies': [
      '<(DEPTH)/base/base.gyp:base',
      '<(DEPTH)/third_party/libxml/libxml.gyp:libxml',
    ],
    'defines': [
      'NOT_HAVE_SA_LEN',
      'USES_NETLINK',
      '__STDC_FORMAT_MACROS',
      'plist_STATIC',
    ],
    'include_dirs': [
      'include',
      'libcnary/include',
    ],
    'direct_dependent_settings': {
      'defines': [
        'UPNP_STATIC_LIB',
        'plist_STATIC',
      ],
      'include_dirs': [
        'include',
      ],
    },
    'export_dependent_settings': [
    ],
    'sources': [
      'libcnary/list.c',
      'libcnary/cnary_node.c',
      'libcnary/node_iterator.c',
      'libcnary/node_list.c',
      'src/plist.c',
      'src/hashtable.c',
      'src/bytearray.c',
      'src/ptrarray.c',
      'src/bplist.c',
#      'src/base64.cc',
      'src/xplist.cc',
      'src/Array.cpp',
      'src/Boolean.cpp',
      'src/Data.cpp',
      'src/Date.cpp',
      'src/Dictionary.cpp',
      'src/Integer.cpp',
      'src/Key.cpp',
      'src/Node.cpp',
      'src/Real.cpp',
      'src/String.cpp',
      'src/Structure.cpp',

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
#'-fexceptions',          
        ],
      }],
      ['OS=="android"', {
        'cflags': [
          '-g',
          '-O2',
          '-Wall',
#'-fexceptions',          
        ],
      }],
      ['OS=="win"', {
        'defines': [
          'WIN32',
          '_LIB',
        ],
        'include_dirs': [
          '<(DEPTH)/rhino/airplay/win32',
        ],
        'msvs_settings': {
          'VCCLCompilerTool': {
            'CompileAs': '2',
          },
        },
        'direct_dependent_settings': {
          'defines': [
            'TARGET_WINDOWS',
            '_VC80_UPGRADE=0x0710',
            '_MBCS',
            '_WINDOWS',
            '_MSVC',
            'WIN32',
            '_WIN32_WINNT=0x0600',
            'NTDDI_VERSION=0x06000000',
            'NOMINMAX',
            '_USE_32BIT_TIME_T',
            'HAS_DX',
            'D3D_DEBUG_INFO',
            '__STDC_CONSTANT_MACROS',
            #'_HAS_ITERATOR_DEBUGGING=0',
            'TAGLIB_STATIC',
            'HAS_WIN32_NETWORK',
          ],
          'include_dirs': [
            '.',
            '<(DEPTH)/rhino/airplay/win32',
          ],
          'linkflags': [
          ],
        },
        'sources!': [  
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
