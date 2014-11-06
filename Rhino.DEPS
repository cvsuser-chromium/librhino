# When adding a new dependency, please update the top-level .gitignore file
# to list the dependency's destination directory.

vars = {
  "rhino_dir": "src/chromium",
  # Use this googlecode_url variable only if there is an internal mirror for it.
  # If you do not know, use the full path while defining your new deps entry.
  "googlecode_url": "http://%s.googlecode.com/svn",
  "sourceforge_url": "http://%(repo)s.svn.sourceforge.net/svnroot/%(repo)s",

  "chromium_trunk": "http://src.chromium.org/svn/trunk/src",
  "chromium_revision": "234326",  

  "webkit_trunk": "http://src.chromium.org/blink/trunk",
  "nacl_trunk": "http://src.chromium.org/native_client/trunk",
  "webkit_revision": "153043",
  "chromium_git": "https://chromium.googlesource.com",
  "chromiumos_git": "https://chromium.googlesource.com/chromiumos",
  "swig_revision": "69281",
  "nacl_revision": "11601",
  # After changing nacl_revision, run 'glient sync' and check native_client/DEPS
  # to update other nacl_*_revision's.
  "nacl_tools_revision": "11437",  # native_client/DEPS: tools_rev
  "gtm_revision": "616",

  "libjingle_revision": "347",
  "libphonenumber_revision": "584",
  "libvpx_revision": "208227",
  "lss_revision": "20",

  # These two FFmpeg variables must be updated together.  One is used for SVN
  # checkouts and the other for Git checkouts.
  "ffmpeg_revision": "203786",
  "ffmpeg_hash": "245a8c0cdfdd5ab3da9045089661017e9ddd8d0e",

  "sfntly_revision": "134",
  "skia_revision": "9712",
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling Skia
  # and V8 without interference from each other.
  "v8_revision": "15255",
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling WebRTC
  # and V8 without interference from each other.
  "webrtc_revision": "4262",
  "jsoncpp_revision": "248",
  "nss_revision": "206843",
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling swarm_client
  # and whatever else without interference from each other.
  "swarm_revision": "207274",
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling openssl
  # and whatever else without interference from each other.
  "openssl_revision": "207965",
}

deps = {

  Var("rhino_dir") + "chromium":
    Var("chromium_trunk") + "@" + Var("chromium_revision"),  
    
  Var("rhino_dir") + "/breakpad/src":
    (Var("googlecode_url") % "google-breakpad") + "/trunk/src@1182",

  Var("rhino_dir") + "/googleurl":
    (Var("googlecode_url") % "google-url") + "/trunk@185",

  Var("rhino_dir") + "/sdch/open-vcdiff":
    (Var("googlecode_url") % "open-vcdiff") + "/trunk@42",

  Var("rhino_dir") + "/testing/gtest":
    (Var("googlecode_url") % "googletest") + "/trunk@629",

  Var("rhino_dir") + "/testing/gmock":
    (Var("googlecode_url") % "googlemock") + "/trunk@410",

  Var("rhino_dir") + "/third_party/angle_dx11":
    Var("chromium_git") +
    "/external/angle.git@f576cb24c8fc02d3f2fb90faebdcf333fb27892f",

  Var("rhino_dir") + "/third_party/trace-viewer":
    (Var("googlecode_url") % "trace-viewer") + "/trunk@653",

#  "src/third_party/WebKit/public":
#    Var("webkit_trunk") + "public@" + Var("webkit_revision"),

  Var("rhino_dir") + "/third_party/icu":
    "/trunk/deps/third_party/icu46@205936",

  Var("rhino_dir") + "/third_party/leveldatabase/src":
    (Var("googlecode_url") % "leveldb") + "/trunk@75",

  Var("rhino_dir") + "/third_party/snappy/src":
    (Var("googlecode_url") % "snappy") + "/trunk@74",

  Var("rhino_dir") + "/tools/grit":
    (Var("googlecode_url") % "grit-i18n") + "/trunk@127",

  Var("rhino_dir") + "/tools/gyp":
    (Var("googlecode_url") % "gyp") + "/trunk@1654",

  Var("rhino_dir") + "/v8":
    (Var("googlecode_url") % "v8") + "/trunk@" + Var("v8_revision"),

  Var("rhino_dir") + "/third_party/sfntly/cpp/src":
    (Var("googlecode_url") % "sfntly") + "/trunk/cpp/src@" +
    Var("sfntly_revision"),

  Var("rhino_dir") + "/third_party/skia/src":
    (Var("googlecode_url") % "skia") + "/trunk/src@" + Var("skia_revision"),

  Var("rhino_dir") + "/third_party/skia/gyp":
    (Var("googlecode_url") % "skia") + "/trunk/gyp@" + Var("skia_revision"),

  Var("rhino_dir") + "/third_party/skia/include":
    (Var("googlecode_url") % "skia") + "/trunk/include@" + Var("skia_revision"),

  Var("rhino_dir") + "/third_party/WebKit/Source":
    Var("webkit_trunk") + "/Source@" + Var("webkit_revision"),

  Var("rhino_dir") + "/third_party/WebKit/public":
    Var("webkit_trunk") + "/public@" + Var("webkit_revision"),
  Var("rhino_dir") + "/third_party/WebKit/Tools":
    Var("webkit_trunk") + "/Tools@" + Var("webkit_revision"),

  Var("rhino_dir") + "/third_party/ots":
    (Var("googlecode_url") % "ots") + "/trunk@102",

  Var("rhino_dir") + "/third_party/v8-i18n":
    (Var("googlecode_url") % "v8-i18n") + "/trunk@191",

  Var("rhino_dir") + "/third_party/libvpx":
    "/trunk/deps/third_party/libvpx@" +
    Var("libvpx_revision"),

  Var("rhino_dir") + "/third_party/ffmpeg":
    "/trunk/deps/third_party/ffmpeg@" +
    Var("ffmpeg_revision"),

  Var("rhino_dir") + "/third_party/libjingle/source":
    (Var("googlecode_url") % "libjingle") + "/trunk@" +
    Var("libjingle_revision"),

  Var("rhino_dir") + "/third_party/yasm/source/patched-yasm":
    "/trunk/deps/third_party/yasm/patched-yasm@167605",

  Var("rhino_dir") + "/third_party/libjpeg_turbo":
    "/trunk/deps/third_party/libjpeg_turbo@177737",

  Var("rhino_dir") + "/third_party/openmax_dl":
    (Var("googlecode_url") % "webrtc") + "/deps/third_party/openmax@4148",

  Var("rhino_dir") + "/third_party/jsoncpp/source/include":
    (Var("sourceforge_url") % {"repo": "jsoncpp"}) +
        "/trunk/jsoncpp/include@" + Var("jsoncpp_revision"),

  Var("rhino_dir") + "/third_party/jsoncpp/source/src/lib_json":
    (Var("sourceforge_url") % {"repo": "jsoncpp"}) +
        "/trunk/jsoncpp/src/lib_json@" + Var("jsoncpp_revision"),

  Var("rhino_dir") + "/third_party/libyuv":
    (Var("googlecode_url") % "libyuv") + "/trunk@723",

  Var("rhino_dir") + "/third_party/smhasher/src":
    (Var("googlecode_url") % "smhasher") + "/trunk@149",

  Var("rhino_dir") + "/third_party/libphonenumber/src/phonenumbers":
     (Var("googlecode_url") % "libphonenumber") +
         "/trunk/cpp/src/phonenumbers@" + Var("libphonenumber_revision"),

  Var("rhino_dir") + "/third_party/opus/src":
    "/trunk/deps/third_party/opus@185324",

  Var("rhino_dir") + "/third_party/mesa/src":
    "/trunk/deps/third_party/mesa@204346",
}


deps_os = {
  "win": {
  },
  "unix": {
    # Linux, really.
    Var("rhino_dir") + "/third_party/openssl":
      "/trunk/deps/third_party/openssl@" + Var("openssl_revision"),

    # Note that this is different from Android's freetype repo.
    Var("rhino_dir") + "/third_party/freetype2/src":
      Var("chromium_git") + "/chromium/src/third_party/freetype2.git" +
      "@d699c2994ecc178c4ed05ac2086061b2034c2178", 
  },
  "android": {
    Var("rhino_dir") + "/third_party/android_tools":
      Var("chromium_git") + "/android_tools.git" +
      "@e9da75d5e88d3e122ac60ee1d642cdcc1acb2bd8",

    Var("rhino_dir") + "/third_party/aosp":
      "/trunk/deps/third_party/aosp@148330",

    Var("rhino_dir") + "/third_party/freetype":
      Var("chromium_git") + "/chromium/src/third_party/freetype.git" +
      "@96551feab72aac26836e9aaf4fae0962d39d5ab0",

    Var("rhino_dir") + "/third_party/guava/src":
      Var("chromium_git") + "/external/guava-libraries.git" +
      "@c523556ab7d0f05afadebd20e7768d4c16af8771",

    Var("rhino_dir") + "/third_party/jarjar":
      "/trunk/deps/third_party/jarjar@170888",

    Var("rhino_dir") + "/third_party/jsr-305/src":
      (Var("googlecode_url") % "jsr-305") + "/trunk@51",
  
    Var("rhino_dir") + "/third_party/openssl":
      "/trunk/deps/third_party/openssl@" + Var("openssl_revision"),

    Var("rhino_dir") + "/third_party/eyesfree/src/android/java/src/com/googlecode/eyesfree/braille":
      (Var("googlecode_url") % "eyes-free") + "/trunk/braille/client/src/com/googlecode/eyesfree/braille@797",
  },
}


include_rules = [
  # Everybody can use some things.
  "+base",
  "+build",
  "+googleurl",
  "+ipc",

  # Everybody can use headers generated by tools/generate_library_loader.
  "+library_loaders",

  "+testing",
  "+third_party/icu/public",
]


# checkdeps.py shouldn't check include paths for files in these dirs:
skip_child_includes = [
  "breakpad",
  "chrome_frame",
  "delegate_execute",
  "googleurl",
  "metro_driver",
  "native_client_sdk",
  "o3d",
  "pdf",
  "sdch",
  "skia",
  "testing",
  "third_party",
  "v8",
  "win8",
]

hooks = [
  {
    # Update LASTCHANGE. This is also run by export_tarball.py in
    # src/tools/export_tarball - please keep them in sync.
    "pattern": ".",
    "action": ["python", Var("rhino_dir") + "/build/util/lastchange.py",
               "-o", Var("rhino_dir") + "/build/util/LASTCHANGE"],
  },
  {
    # Update LASTCHANGE.blink. This is also run by export_tarball.py in
    # src/tools/export_tarball - please keep them in sync.
    "pattern": ".",
    "action": ["python", Var("rhino_dir") + "/build/util/lastchange.py",
               "-s", Var("rhino_dir") + "/third_party/WebKit",
               "-o", Var("rhino_dir") + "/build/util/LASTCHANGE.blink"],
  },
  {
    # A change to a .gyp, .gypi, or to GYP itself should run the generator.
    "pattern": ".",
    "action": ["python", Var("rhino_dir") + "/build/gyp_chromium"],
  },
]
