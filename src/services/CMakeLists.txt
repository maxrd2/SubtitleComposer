set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_MODULE_PATH})

find_package(GStreamer QUIET)
if(GSTREAMER_FOUND)
	set(HAVE_GSTREAMER 1 CACHE INTERNAL EXPORTEDVARIABLE)
	message(STATUS "Found GStreamer: ${GSTREAMER_LIBRARIES} (version ${GSTREAMER_VERSION})")
	add_subdirectory(gstreamer)
else(GSTREAMER_FOUND)
	message(STATUS "GStreamer was not found. GStreamer backend will not be built")
endif(GSTREAMER_FOUND)

add_subdirectory(mplayer)
add_subdirectory(phonon)

macro_optional_find_package(Xine)
if(XINE_FOUND)
	set(HAVE_XINE 1 CACHE INTERNAL EXPORTEDVARIABLE)
	add_subdirectory(xine)
else(XINE_FOUND)
	MESSAGE(STATUS "Xine was not found. Xine backend will not be built")
endif(XINE_FOUND)

set(services_INCLUDE_DIR
	${services_gstreamer_INCLUDE_DIR}
	${services_mplayer_INCLUDE_DIR}
	${services_phonon_INCLUDE_DIR}
	${services_xine_INCLUDE_DIR}
	CACHE INTERNAL EXPORTEDVARIABLE
)

set(services_DEFS
	${services_gstreamer_DEFS}
	${services_mplayer_DEFS}
	${services_phonon_DEFS}
	${services_xine_DEFS}
	CACHE INTERNAL EXPORTEDVARIABLE
)

set(services_LIBS
	${services_gstreamer_LIBS}
	${services_mplayer_LIBS}
	${services_phonon_LIBS}
	${services_xine_LIBS}
	CACHE INTERNAL EXPORTEDVARIABLE
)

set(services_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/service.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/servicebackend.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/player.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/playerbackend.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/videowidget.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/decoder.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/decoderbackend.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/wavewriter.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/waveformat.h
	${services_gstreamer_SRCS}
	${services_mplayer_SRCS}
	${services_phonon_SRCS}
	${services_xine_SRCS}
	CACHE INTERNAL EXPORTEDVARIABLE
)

add_subdirectory(tests)
