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
    'target_name': 'libshairplay',
    'type': 'static_library',
    'dependencies': [
      '<(DEPTH)/base/base.gyp:base',
    ],
    'defines': [
    ],
    'include_dirs': [
      'include',
    ],
    'direct_dependent_settings': {
      'defines': [
      ],
      'include_dirs': [
        'include',
      ],
    },
    'export_dependent_settings': [
    ],
    'sources': [ 
      'src/lib/crypto/bigint.cc',
      'src/lib/crypto/aes.cc', 
      'src/lib/crypto/hmac.cc', 
     'src/lib/crypto/md5.cc', 
      'src/lib/crypto/rc4.cc', 
      'src/lib/crypto/sha1.cc',
      'src/lib/alac/alac.cc',
      'src/lib/base64.cc',
      'src/lib/digest.cc',
      'src/lib/http_parser.cc',
      'src/lib/http_request.cc',
      'src/lib/http_response.cc',
      'src/lib/httpd.cc',
      'src/lib/logger.cc',
      'src/lib/netutils.cc',
      'src/lib/raop.cc',
      'src/lib/raop_buffer.cc',
      'src/lib/raop_rtp.cc',
      'src/lib/rsakey.cc',
      'src/lib/rsapem.cc',
      'src/lib/sdp.cc',
      'src/lib/utils.cc',
      
      #'src/lib/dnssd.c',
      #'src/shairplay.c',
    ],
    'conditions': [
      ['OS=="linux"', {
        'defines': [
          'LINUX_DEFINE',
        ],
        'include_dirs': [
        ],
        'cflags': [
          '-g',
          '-O2',
          '-Wall',
#'-fexceptions',          
        ],
      }],
      ['OS=="android"', {
        'defines': [
          'ANDROID',
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
      ['OS=="win"', {
        'defines': [
          'WIN32',
        ],
        'include_dirs': [
        ],
        'cflags': [
          '/W3',
          '/wd4996',
          '/nologo',
        ],
      }]
    ]
  },
 ]
}
