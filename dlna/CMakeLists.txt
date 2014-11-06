set (current_dir ${HIPPO_SRC_PATH}/modules/dlna)
##48 files.
list (APPEND HIPPO_SRC_CPP
  ${current_dir}/upnp/device/media_renderer.cc
  ${current_dir}/upnp/device/media_server.cc
  ${current_dir}/upnp/device/media_control.cc
  ${current_dir}/upnp/service/avtransport.cc
  ${current_dir}/upnp/service/connection_manager.cc
  ${current_dir}/upnp/service/renderer_control.cc
  ${current_dir}/vendor/services/x_ctc_subscribe.cc
  
  ${current_dir}/dlna_process_host.cc
  ${current_dir}/dlna_process_impl.cc
  ${current_dir}/dlan_thread_impl.cc
  ${current_dir}/dlna_shell.cc
  )
list (APPEND HIPPO_SRC_C

  )

include_directories (
  ${current_dir}/
  ${HIPPO_SRC_PATH}/../third_party/libupnp-1.6.18/
  ${HIPPO_SRC_PATH}/../third_party/libupnp-1.6.18/upnp/inc
  ${HIPPO_SRC_PATH}/../third_party/libupnp-1.6.18/build/inc
  ${HIPPO_SRC_PATH}/../third_party/libupnp-1.6.18/ixml/inc
  )
add_definitions(
  -DINCLUDE_DLNA
  )
if (WIN32)
  ## 18+2 files.
  add_definitions(-DWIN32)
  list ( APPEND HIPPO_TEST_STUB_CPP


    )
  list (APPEND HIPPO_SRC_CPP

    )
else (WIN32)
  #27 files.
  list (APPEND HIPPO_SRC_C

    )
  list (APPEND HIPPO_SRC_CPP
    )

endif (WIN32)
