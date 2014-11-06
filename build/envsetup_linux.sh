SCRIPT_DIR="$(dirname "${BASH_SOURCE:-$0}")"
CURRENT_DIR="$(readlink -f "${SCRIPT_DIR}/../../")"
echo CURRENT_DIR=$CURRENT_DIR

if [[ -z "${CHROME_SRC}" ]]; then
  # If $CHROME_SRC was not set, assume current directory is CHROME_SRC.
  export CHROME_SRC="${CURRENT_DIR}"
fi
if [[ "${CURRENT_DIR/"${CHROME_SRC}"/}" == "${CURRENT_DIR}" ]]; then
  # If current directory is not in $CHROME_SRC, it might be set for other
  # source tree. If $CHROME_SRC was set correctly and we are in the correct
  # directory, "${CURRENT_DIR/"${CHROME_SRC}"/}" will be "".
  # Otherwise, it will equal to "${CURRENT_DIR}"
  echo "Warning: Current directory is out of CHROME_SRC, it may not be \
the one you want."
  echo "${CHROME_SRC}"
fi
################################################################################
# To build rhino, we use the Android build system and build inside an Android
# source tree. This method is called from non_sdk_build_init() and adds to the
# settings specified there.
#############################################################################
linux_build_init() {
  export GYP_DEFINES="${DEFINES}"
  export GYP_GENERATORS="ninja"
  export CHROMIUM_GYP_FILE="${CHROME_SRC}/rhino/rhino.gyp"
}

rhino_linux_gyp() {
  linux_build_init
  # This is just a simple wrapper of gyp_chromium, please don't add anything
  # in this function.
  echo "GYP_GENERATORS set to '$GYP_GENERATORS'"
  (
    "${CHROME_SRC}/build/gyp_chromium" --depth="${CHROME_SRC}" --check "$@"
  )
}
rhino_linux_build() {
  ninja -C $@
}
