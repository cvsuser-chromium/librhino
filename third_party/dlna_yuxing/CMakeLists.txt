
message(" ")
message("!!!!! NOTE: compile BigDlna including dtcpip, ffmpeg, json, libupnp, and dlna")
message(" ")
#if(ENABLE_DLNA_OVERSEA)
#add_definitions(-DDLNA_OVERSEA)
#endif(ENABLE_DLNA_OVERSEA)

include(DlnaCommonConfig.cmake)

project(BigDlna)
cmake_minimum_required(VERSION 2.6)


#add_subdirectory(dtcpip)
add_subdirectory(ffmpeg)
#add_subdirectory(json)
add_subdirectory(libupnp)
add_subdirectory(dlna)



