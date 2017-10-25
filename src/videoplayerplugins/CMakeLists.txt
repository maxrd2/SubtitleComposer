set(videoplayerplugins_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/../videoplayer/videoplayer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/../videoplayer/playerbackend.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/../videoplayer/videowidget.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/../scconfigdummy.cpp
	CACHE INTERNAL EXPORTEDVARIABLE
)

add_subdirectory(gstreamer)
add_subdirectory(mpv)
add_subdirectory(mplayer)
add_subdirectory(phonon)
add_subdirectory(xine)
