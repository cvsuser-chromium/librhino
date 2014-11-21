# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This is all.gyp file for Android to prevent breakage in Android and other
# platform; It will be churning a lot in the short term and eventually be merged
# into all.gyp.

{
  'variables': {
    # A hook that can be overridden in other repositories to add additional
    # compilation targets to 'All'
    'android_app_targets%': [],
  },
  'includes': [
    'synclib.gypi',
    'rhino_win32.gypi',
    'airplay.gypi',
  ],
  'targets': [
    {
      'target_name': 'All',
      'type': 'none',
      'dependencies': [
        'synclib',
        '<(DEPTH)/jingle/jingle.gyp:jingle_unittests',
#        '<(DEPTH)/rhino/airplay/airplay.gyp:airplay',
#        '<@(android_app_targets)',
      ],
      'conditions': [
        ['OS=="android"', {
          'dependencies': [
#            '<(DEPTH)/content/content.gyp:content',
#            '<(DEPTH)/content/content.gyp:content_shell_apk',
#            '<(DEPTH)/rhino/android/rhino_app.gyp:hybroad_media',
          ],
        }],
        ['OS=="linux"', {
          'dependencies': [
          ],
        }],
        ['OS=="win"', {
          'dependencies': [
            'rhino.elf',
          ],
        }],
      ],
    }, # target_name: All
  ],  # targets
}
