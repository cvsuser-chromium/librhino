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
    'target_name': 'mdnsresponder',
    'type': 'static_library',
    'dependencies': [
      '<(DEPTH)/base/base.gyp:base',
    ],
    'defines': [
      'NOT_HAVE_SA_LEN',
      'USES_NETLINK',
    ],
    'include_dirs': [
      'mDNSShared',
      'mDNSCore',
    ],
    'direct_dependent_settings': {
      'defines': [
        'UPNP_STATIC_LIB',
      ],
      'include_dirs': [
        'mDNSShared',
        'mDNSCore',
      ],
    },
    'export_dependent_settings': [
    ],
    'sources': [
      'mDNSCore/CryptoAlg.c', 
      'mDNSCore/DNSCommon.c',
      'mDNSCore/DNSDigest.cc',
      'mDNSCore/mDNS.c',
      'mDNSCore/mDnsEmbedded.c',
      'mDNSCore/uDNS.c',
      'mDNSShared/dnssd_clientlib.c',
      'mDNSShared/dnssd_clientshim.c',
      'mDNSShared/GenLinkedList.c',
      'mDNSShared/mDNSDebug.c',
      'mDNSShared/PlatformCommon.cc',
      'mDNSShared/DebugServices.c',
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
        'sources': [  
          'mDNSPosix/mDNSPosix.c',
          'mDNSPosix/mDNSUNP.c',
        ],
      }],
      ['OS=="win"', {
        'defines': [
          'WIN32',
        ],
        'include_dirs': [
          '<(DEPTH)/rhino/airplay/win32',
        ],
        'cflags': [
          '/W3',
          '/wd4996',
          '/nologo',
        ],
        'sources': [
          'mDNSWindows/Poll.c',
          'mDNSWindows/Secret.c',
          'mDNSWindows/mDNSWin32.cpp',
          'mDNSWindows/SystemService/Firewall.cpp',
        ],
        'direct_dependent_settings': {
          'msvs_settings': {
            'VCLinkerTool': {
              'AdditionalDependencies': [
                'Netapi32.lib',
              ],
            },
          },
        },
      }]
    ]
  },
 ]
}
