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
          'msvs_configuration_attributes': {
            #'CharacterSet': '0',
          },
        },
        'Release': {
          'msvs_configuration_attributes': {
            #'CharacterSet': '0',
          },
        },
      },
      'target_name': 'airplay',
      'type': '<(component)',
      'dependencies': [
        '<(DEPTH)/rhino/airplay/libplist/libplist.gyp:libplist',
        '<(DEPTH)/rhino/airplay/mdnsresponder/mdnsresponder.gyp:mdnsresponder',
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/net/net.gyp:net',
        '<(DEPTH)/url/url.gyp:url_lib',
        '<(DEPTH)/rhino/third_party/libshairplay/libshairplay.gyp:libshairplay',
        '<(DEPTH)/rhino/third_party/libPlatinum/libPlatinum.gyp:libPlatinum',
        '<(DEPTH)/rhino/third_party/tinyxml/tinyxml.gyp:tinyxml',
        '<(DEPTH)/rhino/third_party/json-c/json_c.gyp:json_c',
      ],
      'defines': [
        'HAS_ZEROCONF',
        'HAS_AIRPLAY',
        'HAS_AIRTUNES',
        #'HAS_UPNP',
      ],
      'include_dirs': [
        '.',
        '<(DEPTH)/testing/gtest/include',
      ],
      'direct_dependent_settings': {
        'defines': [
          'HAS_ZEROCONF',
          'HAS_AIRPLAY',
          'HAS_AIRTUNES',
          #'HAS_UPNP',
        ],
        'include_dirs': [
          '.',
        ],
        'linkflags': [
        ],
      },
      'export_dependent_settings': [
        '<(DEPTH)/base/base.gyp:base',
      ],
      'sources': [
        'commons/ilog.cpp',
        'commons/Exception.cpp',
        'interfaces/AnnouncementManager.cpp',
        'network/airplay_server.cc',
        #'network/AirPlayServer_newer.cpp',
        'network/AirTunesServer.cpp',
        'network/DNSNameCache.cpp',
        'network/Zeroconf.cpp',
        'network/ZeroconfBrowser.cpp',
        'network/Network.cpp',
        'network/NetworkServices.cpp',
        'network/upnp/UPnP.cpp',
        'network/upnp/UPnPInternal.cpp',
        #'network/upnp/UPnPPlayer.cpp',
        'network/upnp/UPnPRenderer.cpp',
        'network/upnp/UPnPServer.cpp',
        'network/upnp/UPnPSettings.cpp',
        'network/mdns/ZeroconfMDNS.cpp',
        'network/mdns/ZeroconfBrowserMDNS.cpp',
        'threads/Atomics.cpp',
        'threads/Event.cpp',
        'threads/LockFree.cpp',
        'threads/SystemClock.cpp',
        'threads/Thread.cpp',
        'threads/Timer.cpp',
        'utils/HttpParser.cpp',
        'utils/JobManager.cpp',
        'utils/log.cpp',
        'utils/EndianSwap.cpp',
        #'utils/URIUtils.cpp',        
        'utils/Variant.cpp',
        'settings/AdvancedSettings.cpp',
        'Application.cpp',
        'FileItem.cpp',
        #'URL.cpp',
        #'Util.cpp',
      ],
      'conditions': [
        ['OS=="linux"', {
          'defines': [
            'TARGET_POSIX',
            'TARGET_LINUX',
            'HAS_LINUX_NETWORK',
          ],
          'dependencies': [
          ],
          'include_dirs': [
            '<(DEPTH)/rhino/airplay/linux',
          ],
          'sources': [
            'network/linux/NetworkLinux.cpp',
            'threads/platform/pthreads/Implementation.cpp',
            'network/airplay_server_posix.cc',
          ],
          'cflags': [
            '-g',
            '-O2',
            '-Wall',
            '-fexceptions',
          ],          
        }],
        ['OS=="android"', {
          'defines': [
            'TARGET_POSIX',
            'TARGET_ANDROID',
            'HAS_LINUX_NETWORK',
          ],
          'dependencies': [
          ],
          'include_dirs': [
            '<(DEPTH)/rhino/airplay/linux',
          ],
          'sources': [
            'android/bionic_supplement/getdelim.c',
            'network/linux/NetworkLinux.cpp',
            'threads/platform/pthreads/Implementation.cpp',
            'network/airplay_server_posix.cc',
          ],
          'cflags': [
            '-g',
            '-O2',
            '-Wall',
#            '-fexceptions',
          ],
          'direct_dependent_settings': {
            'defines': [
              'TARGET_POSIX',
              'TARGET_ANDROID',
            ],
            'include_dirs': [
              '<(DEPTH)/rhino/airplay/linux',
            ],
          },
        }],
        ['OS=="win"', {
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
            'TAGLIB_STATIC',
            'HAS_WIN32_NETWORK',
          ],
          'include_dirs': [
            '<(DEPTH)/rhino/airplay/win32',
          ],
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
          'cflags': [
            '/W3',
            '/wd4996',
            '/nologo',
          ],
          'sources': [
            'network/airplay_server_win.cc',
            'network/windows/NetworkWin32.cpp',
            'threads/platform/win/Win32Exception.cpp',
          ],
        }],
      ],
    },
  ],
}

