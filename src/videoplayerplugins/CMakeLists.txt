set(videoplayerplugins_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/../videoplayer/videoplayer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/../videoplayer/playerbackend.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/../videoplayer/videowidget.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/scconfigdummy.cpp
	CACHE INTERNAL EXPORTEDVARIABLE
)

find_package(GStreamer QUIET)
find_package(GStreamerPlugins QUIET)
if(GSTREAMER_FOUND AND GSTREAMER_PLUGIN_VIDEO_FOUND)
	set(HAVE_GSTREAMER 1 CACHE INTERNAL EXPORTEDVARIABLE)
	message(STATUS "Found GStreamer: ${GSTREAMER_LIBRARIES} (version ${GSTREAMER_VERSION})")
	add_subdirectory(gstreamer)
else(GSTREAMER_FOUND AND GSTREAMER_PLUGIN_VIDEO_FOUND)
	message(STATUS "GStreamer and/or GStreamer Video was not found. GStreamer backend will not be built")
endif(GSTREAMER_FOUND AND GSTREAMER_PLUGIN_VIDEO_FOUND)

find_package(MPV 1.14 QUIET)
if(MPV_FOUND)
	set(HAVE_MPV 1 CACHE INTERNAL EXPORTEDVARIABLE)
	message(STATUS "Found LibMPV: ${MPV_LIBRARIES} (version ${MPV_VERSION})")
	add_subdirectory(mpv)
else(MPV_FOUND)
	message(STATUS "LibMPV v1.14+ was not found. MPV backend will not be built. (Found version ${MPV_VERSION})")
endif(MPV_FOUND)

add_subdirectory(mplayer)

add_subdirectory(phonon)

find_package(Xine)
find_package(X11 QUIET)
if(XINE_FOUND AND X11_FOUND)
	set(HAVE_XINE 1 CACHE INTERNAL EXPORTEDVARIABLE)
	add_subdirectory(xine)
else(XINE_FOUND AND X11_FOUND)
	message(STATUS "Xine/X11 was not found. Xine backend will not be built")
endif(XINE_FOUND AND X11_FOUND)
