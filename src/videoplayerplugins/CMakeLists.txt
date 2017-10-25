set(videoplayerplugins_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/../videoplayer/videoplayer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/../videoplayer/playerbackend.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/../videoplayer/videowidget.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/../scconfigdummy.cpp
	CACHE INTERNAL EXPORTEDVARIABLE
)

add_subdirectory(gstreamer)

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

find_package(Xine 1.1.9 QUIET)
find_package(X11 QUIET)
if(XINE_FOUND AND X11_FOUND)
	set(HAVE_XINE 1 CACHE INTERNAL EXPORTEDVARIABLE)
	message(STATUS "Found Xine: ${XINE_LIBRARY} (version ${XINE_VERSION})")
	add_subdirectory(xine)
else(XINE_FOUND AND X11_FOUND)
	message(STATUS "Xine v1.1.9+ and/or LibX11 was not found. Xine backend will not be built. (Found Xine version ${XINE_VERSION})")
endif(XINE_FOUND AND X11_FOUND)
