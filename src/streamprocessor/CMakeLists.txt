find_package(GStreamer REQUIRED QUIET)
find_package(GStreamerPlugins REQUIRED QUIET)
if(GSTREAMER_FOUND AND GSTREAMER_PLUGIN_AUDIO_FOUND)
	set(HAVE_GSTREAMER 1 CACHE INTERNAL EXPORTEDVARIABLE)
	message(STATUS "Found GStreamer: ${GSTREAMER_LIBRARIES} (version ${GSTREAMER_VERSION})")
else(GSTREAMER_FOUND AND GSTREAMER_PLUGIN_AUDIO_FOUND)
	message(ERROR "GStreamer and/or GStreamer Audio was not found.")
endif(GSTREAMER_FOUND AND GSTREAMER_PLUGIN_AUDIO_FOUND)

set(streamprocessor_INCLUDE_DIR
	${streamprocessor_gstreamer_INCLUDE_DIR}
	CACHE INTERNAL EXPORTEDVARIABLE
)

set(streamprocessor_DEFS
	${streamprocessor_gstreamer_DEFS}
	CACHE INTERNAL EXPORTEDVARIABLE
)

set(streamprocessor_LIBS
	${streamprocessor_gstreamer_LIBS}
	CACHE INTERNAL EXPORTEDVARIABLE
)

set(streamprocessor_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/streamprocessor.cpp
	${streamprocessor_gstreamer_SRCS}
	CACHE INTERNAL EXPORTEDVARIABLE
)
