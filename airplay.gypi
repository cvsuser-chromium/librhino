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
        'airplay/commons/ilog.cpp',
        'airplay/commons/Exception.cpp',
        'airplay/interfaces/AnnouncementManager.cpp',
        'airplay/network/airplay_server.cc',
        #'network/AirPlayServer_newer.cpp',
        'airplay/network/AirTunesServer.cpp',
        'airplay/network/DNSNameCache.cpp',
        'airplay/network/Zeroconf.cpp',
        'airplay/network/ZeroconfBrowser.cpp',
        'airplay/network/Network.cpp',
        'airplay/network/NetworkServices.cpp',
        'airplay/network/upnp/UPnP.cpp',
        'airplay/network/upnp/UPnPInternal.cpp',
        #'airplay/network/upnp/UPnPPlayer.cpp',
        'airplay/network/upnp/UPnPRenderer.cpp',
        'airplay/network/upnp/UPnPServer.cpp',
        'airplay/network/upnp/UPnPSettings.cpp',
        'airplay/network/mdns/ZeroconfMDNS.cpp',
        'airplay/network/mdns/ZeroconfBrowserMDNS.cpp',
        'airplay/threads/Atomics.cpp',
        'airplay/threads/Event.cpp',
        'airplay/threads/LockFree.cpp',
        'airplay/threads/SystemClock.cpp',
        'airplay/threads/Thread.cpp',
        'airplay/threads/Timer.cpp',
        'airplay/utils/HttpParser.cpp',
        'airplay/utils/JobManager.cpp',
        'airplay/utils/log.cpp',
        'airplay/utils/EndianSwap.cpp',
        #'utils/URIUtils.cpp',        
        'airplay/utils/Variant.cpp',
        'airplay/settings/AdvancedSettings.cpp',
        'airplay/Application.cpp',
        'airplay/FileItem.cpp',
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
            'airplay/network/linux/NetworkLinux.cpp',
            'airplay/threads/platform/pthreads/Implementation.cpp',
            'airplay/network/airplay_server_posix.cc',
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
            'airplay/android/bionic_supplement/getdelim.c',
            'airplay/network/linux/NetworkLinux.cpp',
            'airplay/threads/platform/pthreads/Implementation.cpp',
            'airplay/network/airplay_server_posix.cc',
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
            'airplay/network/airplay_server_win.cc',
            'airplay/network/windows/NetworkWin32.cpp',
            'airplay/threads/platform/win/Win32Exception.cpp',
          ],
        }],
      ],
    },
  ],
}
