{
  'includes': [
    'android_common.gypi',
  ],
  'targets': [
  {
    'target_name': 'media_jni_headers',
    'type': 'none',
    'dependencies': [
    ],
    'direct_dependent_settings': {
      'include_dirs': [
        '<(SHARED_INTERMEDIATE_DIR)/hybroad_media',
      ],
    },
    'includes': [ 'rhino_jni_header.gypi' ],
  },
  {
    'target_name': 'rhino_jni',
    'type': 'shared_library',
    'dependencies': [
      'media_jni_headers',
      '<(DEPTH)/base/base.gyp:base',
      '<(DEPTH)/rhino/dlna/dlna.gyp:dlna',
      '<(DEPTH)/rhino/airplay/airplay.gyp:airplay',
    ],
    'include_dirs': [
      '../dlna/android',
      '../include',
      '<(DEPTH)/rhino/third_party/dlna_yuxing/dlna/common',
      '<(DEPTH)/rhino/third_party/libupnp-1.6.18/upnp/inc',
      '<(DEPTH)/rhino/third_party/libupnp-1.6.18/ixml/inc',
      '<(DEPTH)/rhino/third_party/libupnp-1.6.18/build/inc',
    ],
    'cflags': [
      '<@(android_system_headers)',
    ],
    'cflags_cc': [
      #'-frtti',
    ],
    'ldflags': [
      '-L<(android_product_out)/system/lib',
      '<@(android_system_libs)',
    ],
    'direct_dependent_settings': {
      'defines': [
      ],
      'include_dirs': [
        '.',
      ],
    },
    'sources': [
      'app/jni/android_dlna_DMPDevice.cpp',
      'app/jni/android_dlna_DMRDevice.cpp',
      'app/jni/rhino_jni_registrar.cc',
      'app/jni/airplay_client.cc',
    ],
  },
  {
    'target_name': 'hybroad_media',
    'type': 'none',
    'dependencies': [
      'rhino_jni',
      '<(DEPTH)/base/base.gyp:base_java',
    ],
    'variables': {
      'java_in_dir': 'app',
      'has_java_resources': 1,
      'R_package': 'com.hybroad.media',
      'R_package_relpath': 'com/hybroad/media',
      'input_jars_paths': [
        'app/lib/antlr-2.7.4.jar',
        'app/lib/chardet-1.0.jar',
        'app/lib/cpdetector_1.0.10.jar',
        'app/lib/jargs-1.0.jar'
      ],
    },
    'includes': [ '../../build/java.gypi' ],
  },
  {
    'target_name': 'hybroad_media_apk',
    'type': 'none',
    'dependencies': [
      'rhino_jni',
#'<(DEPTH)/rhino/dlna/dlna.gyp:libdlna_service',
#'<(DEPTH)/rhino/dlna/dlna.gyp:dlnaserver.elf',
    ],
    'variables': {
      'apk_name': 'hybroad_media',
      'java_in_dir': 'app',
      'resource_dir': 'app/res',
      'native_lib_target': 'librhino_jni',
      'input_jars_paths': [
        'app/lib/antlr-2.7.4.jar',
        'app/lib/chardet-1.0.jar',
        'app/lib/cpdetector_1.0.10.jar',
        'app/lib/jargs-1.0.jar'
      ],
    #  'additional_input_paths': ['<(PRODUCT_DIR)/content_shell/assets/content_shell.pak'],
    #  'asset_location': '<(ant_build_out)/content_shell/assets',
    },
    'includes': [ '../../build/java_apk.gypi' ],
  },]
}
