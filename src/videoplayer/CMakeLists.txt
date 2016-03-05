find_package(GStreamer QUIET)
find_package(GStreamerPlugins QUIET)
if(GSTREAMER_FOUND AND GSTREAMER_PLUGIN_VIDEO_FOUND)
	set(HAVE_GSTREAMER 1 CACHE INTERNAL EXPORTEDVARIABLE)
	message(STATUS "Found GStreamer: ${GSTREAMER_LIBRARIES} (version ${GSTREAMER_VERSION})")
	add_subdirectory(gstreamer)
else(GSTREAMER_FOUND AND GSTREAMER_PLUGIN_VIDEO_FOUND)
	message(STATUS "GStreamer and/or GStreamer Video was not found. GStreamer backend will not be built")
endif(GSTREAMER_FOUND AND GSTREAMER_PLUGIN_VIDEO_FOUND)

find_package(MPV QUIET)
if(MPV_FOUND)
	set(HAVE_MPV 1 CACHE INTERNAL EXPORTEDVARIABLE)
	message(STATUS "Found MPV: ${MPV_LIBRARIES} (version ${MPV_VERSION})")
	add_subdirectory(mpv)
else(MPV_FOUND)
	message(STATUS "MPV was not found. MPV backend will not be built")
endif(MPV_FOUND)

add_subdirectory(mplayer)

add_subdirectory(phonon)

find_package(Xine)
if(XINE_FOUND)
	set(HAVE_XINE 1 CACHE INTERNAL EXPORTEDVARIABLE)
	add_subdirectory(xine)
else(XINE_FOUND)
	message(STATUS "Xine was not found. Xine backend will not be built")
endif(XINE_FOUND)

set(videoplayer_INCLUDE_DIR
	${videoplayer_gstreamer_INCLUDE_DIR}
	${videoplayer_mplayer_INCLUDE_DIR}
	${videoplayer_mpv_INCLUDE_DIR}
	${videoplayer_phonon_INCLUDE_DIR}
	${videoplayer_xine_INCLUDE_DIR}
	CACHE INTERNAL EXPORTEDVARIABLE
)

set(videoplayer_DEFS
	${videoplayer_gstreamer_DEFS}
	${videoplayer_mplayer_DEFS}
	${videoplayer_mpv_DEFS}
	${videoplayer_phonon_DEFS}
	${videoplayer_xine_DEFS}
	CACHE INTERNAL EXPORTEDVARIABLE
)

set(videoplayer_LIBS
	${videoplayer_gstreamer_LIBS}
	${videoplayer_mplayer_LIBS}
	${videoplayer_mpv_LIBS}
	${videoplayer_phonon_LIBS}
	${videoplayer_xine_LIBS}
	CACHE INTERNAL EXPORTEDVARIABLE
)

set(videoplayer_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/videoplayer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/playerbackend.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/videowidget.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/waveformat.h
	${videoplayer_gstreamer_SRCS}
	${videoplayer_mplayer_SRCS}
	${videoplayer_mpv_SRCS}
	${videoplayer_phonon_SRCS}
	${videoplayer_xine_SRCS}
	CACHE INTERNAL EXPORTEDVARIABLE
)
