SCRIPT_DIR="$(dirname "${BASH_SOURCE:-$0}")"
CURRENT_DIR="$(readlink -f "${SCRIPT_DIR}/../../")"

. "${CURRENT_DIR}"/build/android/envsetup.sh

################################################################################
# Exports environment variables common to both sdk and non-sdk build (e.g. PATH)
# based on CHROME_SRC and ANDROID_TOOLCHAIN, along with DEFINES for GYP_DEFINES.
################################################################################
common_vars_defines_rhino() {
  
  common_check_toolchain

  # Add Android SDK/NDK tools to system path.
  export PATH=$PATH:${ANDROID_NDK_ROOT}
  export PATH=$PATH:${ANDROID_SDK_ROOT}/tools
  export PATH=$PATH:${ANDROID_SDK_ROOT}/platform-tools

  # Must have tools like arm-linux-androideabi-gcc on the path for ninja
  export PATH=$PATH:${ANDROID_TOOLCHAIN}

  # Add Chromium Android development scripts to system path.
  # Must be after CHROME_SRC is set.
  export PATH=$PATH:${CHROME_SRC}/build/android

  # TODO(beverloo): Remove these once all consumers updated to --strip-binary.
  export OBJCOPY=$(echo ${ANDROID_TOOLCHAIN}/*-objcopy)
  export STRIP=$(echo ${ANDROID_TOOLCHAIN}/*-strip)

  # The set of GYP_DEFINES to pass to gyp. Use 'readlink -e' on directories
  # to canonicalize them (remove double '/', remove trailing '/', etc).
  DEFINES="OS=android"
  DEFINES+=" host_os=${host_os}"
  DEFINES+=" gcc_version=${gcc_version}"

  # The order file specifies the order of symbols in the .text section of the
  # shared library, libchromeview.so.  The file is an order list of section
  # names and the library is linked with option
  # --section-ordering-file=<orderfile>. The order file is updated by profiling
  # startup after compiling with the order_profiling=1 GYP_DEFINES flag.
  ORDER_DEFINES="order_text_section=${CHROME_SRC}/orderfiles/orderfile.out"

  # The following defines will affect ARM code generation of both C/C++ compiler
  # and V8 mksnapshot.
  case "${TARGET_ARCH}" in
    "arm")
      DEFINES+=" ${ORDER_DEFINES}"
      DEFINES+=" target_arch=arm"
      ;;
    "x86")
    # TODO(tedbo): The ia32 build fails on ffmpeg, so we disable it here.
      DEFINES+=" use_libffmpeg=0"

      host_arch=$(uname -m | sed -e \
        's/i.86/ia32/;s/x86_64/x64/;s/amd64/x64/;s/arm.*/arm/;s/i86pc/ia32/')
      DEFINES+=" host_arch=${host_arch}"
      DEFINES+=" target_arch=ia32"
      ;;
    "mips")
      DEFINES+=" target_arch=mipsel"
      ;;
    *)
      echo "TARGET_ARCH: ${TARGET_ARCH} is not supported." >& 2
      print_usage
      return 1
  esac
}

################################################################################
# To build rhino using the Android build system and build inside an Android
# source tree. 
#############################################################################
android_source_build_init() {
  gcc_version=46
  ANDROID_SDK_VERSION=17
  ANDROID_STLPORT_ROOT=${ANDROID_BUILD_TOP}/external/stlport/stlport  
  # For the WebView build we always use the NDK and SDK in the Android tree,
  # and we don't touch ANDROID_TOOLCHAIN which is already set by Android.
  export ANDROID_NDK_ROOT=${ANDROID_BUILD_TOP}/prebuilt/ndk/android-ndk-r6
  export ANDROID_SDK_ROOT=${ANDROID_BUILD_TOP}/prebuilt/sdk/${ANDROID_SDK_VERSION}

  # We need to supply SDK paths relative to the top of the Android tree to make
  # sure the generated Android makefiles are portable, as they will be checked
  # into the Android tree.
  ANDROID_SDK=$(python -c \
      "import os.path; print os.path.relpath('${ANDROID_SDK_ROOT}', \
      '${ANDROID_BUILD_TOP}')")
  case "${host_os}" in
    "linux")
      ANDROID_SDK_TOOLS=$(python -c \
          "import os.path; \
          print os.path.relpath('${ANDROID_SDK_ROOT}/../tools/linux', \
          '${ANDROID_BUILD_TOP}')")
      ;;
  esac
  common_vars_defines_rhino

  DEFINES+=" werror="
  DEFINES+=" android_webview_build=1"
  # temporary until all uses of android_build_type are gone (crbug.com/184431)
  DEFINES+=" android_build_type=1"
  DEFINES+=" android_src=\$(ANDROID_BUILD_TOP)"
  DEFINES+=" android_source_tree=\$(ANDROID_BUILD_TOP)"
  DEFINES+=" android_sdk=\$(ANDROID_BUILD_TOP)/${ANDROID_SDK}"
  DEFINES+=" android_sdk_root=\$(ANDROID_BUILD_TOP)/${ANDROID_SDK}"
  DEFINES+=" android_sdk_tools=\$(ANDROID_BUILD_TOP)/${ANDROID_SDK_TOOLS}"
  DEFINES+=" android_sdk_version=${ANDROID_SDK_VERSION}"
  DEFINES+=" android_toolchain=${ANDROID_TOOLCHAIN}" 
  
  export GYP_DEFINES="${DEFINES}"

  export GYP_GENERATORS="android"

  export GYP_GENERATOR_FLAGS="${GYP_GENERATOR_FLAGS} default_target=All"
  export GYP_GENERATOR_FLAGS="${GYP_GENERATOR_FLAGS} limit_to_target_all=1"
  export GYP_GENERATOR_FLAGS="${GYP_GENERATOR_FLAGS} auto_regeneration=0"

  export CHROMIUM_GYP_FILE="${CHROME_SRC}/rhino/rhino.gyp"

  echo GYP_DEFINES=$DEFINES
  echo ANDROID_NDK_ROOT=$ANDROID_NDK_ROOT
}

################################################################################
# Initializes environment variables for NDK/SDK build. Only Android NDK Revision
# 7 on Linux or Mac is offically supported. To run this script, the system
# environment ANDROID_NDK_ROOT must be set to Android NDK's root path.  The
# ANDROID_SDK_ROOT only needs to be set to override the default SDK which is in
# the tree under $ROOT/src/third_party/android_tools/sdk.
# To build Chromium for Android with NDK/SDK follow the steps below:
#  > export ANDROID_NDK_ROOT=<android ndk root>
#  > export ANDROID_SDK_ROOT=<android sdk root> # to override the default sdk
#  > . build/android/envsetup.sh
#  > make
################################################################################
non_android_source_build_init() {
  gcc_version=46
  ANDROID_SDK_VERSION=17
  #ANDROID_STLPORT_ROOT=${ANDROID_BUILD_TOP}/external/stlport/stlport  
  # For the WebView build we always use the NDK and SDK in the Android tree,
  # and we don't touch ANDROID_TOOLCHAIN which is already set by Android.
  export ANDROID_NDK_ROOT=${CHROME_SRC}/third_party/android_tools/ndk
  export ANDROID_SDK_ROOT=${CHROME_SRC}/third_party/android_tools/sdk
  #export ANDROID_SDK_TOOLS=${ANDROID_SDK_ROOT}/platform-tools

  common_vars_defines_rhino

  #DEFINES+=" werror="
  DEFINES+=" android_webview_build=0"
  # temporary until all uses of android_build_type are gone (crbug.com/184431)
  DEFINES+=" android_build_type=1"
  DEFINES+=" android_src=${ANDROID_BUILD_TOP}"
  DEFINES+=" android_source_tree=${ANDROID_BUILD_TOP}"
  #DEFINES+=" android_sdk=${ANDROID_BUILD_TOP}/${ANDROID_SDK}"
  #DEFINES+=" android_sdk_root=${ANDROID_BUILD_TOP}/${ANDROID_SDK}"
  #DEFINES+=" android_sdk_tools=${ANDROID_BUILD_TOP}/${ANDROID_SDK_TOOLS}"
  DEFINES+=" android_sdk_version=${ANDROID_SDK_VERSION}"
  DEFINES+=" android_toolchain=${ANDROID_TOOLCHAIN}" 
  #DEFINES+=" component=shared_library" 
  DEFINES+=" android_product_out=${ANDROID_PRODUCT_OUT}"

  export GYP_DEFINES="${DEFINES}"

  export GYP_GENERATORS="ninja"

  export GYP_GENERATOR_FLAGS="${GYP_GENERATOR_FLAGS} default_target=All"
  export GYP_GENERATOR_FLAGS="${GYP_GENERATOR_FLAGS} limit_to_target_all=1"
  export GYP_GENERATOR_FLAGS="${GYP_GENERATOR_FLAGS} auto_regeneration=0"
  export CHROMIUM_GYP_FILE="${CHROME_SRC}/rhino/rhino.gyp"

  echo GYP_DEFINES=$DEFINES
  echo ANDROID_NDK_ROOT=$ANDROID_NDK_ROOT
}

rhino_android_gyp() {
  #if [ '@1'x == "ninja"x ]; then
  android_source_build_init
  #else
  #  android_source_build_init
  #fi
  # This is just a simple wrapper of gyp_chromium, please don't add anything
  # in this function.
  echo "GYP_GENERATORS set to '$GYP_GENERATORS'"
  echo "build rhino with android build system in android source tree."
  (
    "${CHROME_SRC}/build/gyp_chromium" --depth="${CHROME_SRC}" --check "$@"
  )
}
rhino_ninja_gyp() {  
  non_android_source_build_init
  #sdk_build_init
  #export CHROMIUM_GYP_FILE="${CHROME_SRC}/rhino/rhino.gyp"
  # This is just a simple wrapper of gyp_chromium, please don't add anything
  # in this function.
  echo "GYP_GENERATORS set to '$GYP_GENERATORS'"
  echo "build rhino out of android source tree."
  (
    "${CHROME_SRC}/build/gyp_chromium" --depth="${CHROME_SRC}" --check "$@"
  )
}
rhino_build() {
  cd $ANDROID_BUILD_TOP 
  mmm $CHROME_SRC $@
  cd $CHROME_SRC
}
