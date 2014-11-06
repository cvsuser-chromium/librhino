# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  # TODO(jrg): Update this action and other jni generators to only
  # require specifying the java directory and generate the rest.
  # TODO(jrg): when doing the above, make sure we support multiple
  # output directories (e.g. browser/jni and common/jni if needed).
  'sources': [
    'app/src/com/hybroad/dlna/AirPlayClient.java',
    'app/src/com/hybroad/dlna/DMPDevice.java',
    'app/src/com/hybroad/dlna/DMRDeviceClient.java',
   ],
  'variables': {
    'jni_gen_package': 'hybroad_media'
  },
  'includes': [ '../../build/jni_generator.gypi' ],
}
