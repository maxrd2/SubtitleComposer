find_package(GLIB2)
find_package(GObject)
#find_package(LibXml2)
find_package(GStreamer)
find_package(GStreamerPlugins)

if(NOT GLIB2_FOUND OR NOT GOBJECT_FOUND OR NOT GSTREAMER_FOUND OR NOT GSTREAMER_PLUGIN_VIDEO_FOUND)
	message(STATUS "Have NOT Found GLib/GObject/GStreamer/GStreamerVideo - GStreamer videoplayer plugin will not be built")
	return()
endif(NOT GLIB2_FOUND OR NOT GOBJECT_FOUND OR NOT GSTREAMER_FOUND OR NOT GSTREAMER_PLUGIN_VIDEO_FOUND)

set(videoplayer_gstreamer_INCLUDE_DIR
	${GLIB2_INCLUDE_DIR}
	${GSTREAMER_INCLUDE_DIR}
	${GSTREAMER_VIDEO_PLUGINNAME_INCLUDE_DIR}
	CACHE INTERNAL EXPORTEDVARIABLE
)
set(videoplayer_gstreamer_DEFS
	${GOBJECT_DEFINITIONS}
	CACHE INTERNAL EXPORTEDVARIABLE
)
set(videoplayer_gstreamer_LIBS
	${GLIB2_LIBRARIES}
	${GOBJECT_LIBRARIES}
	${GSTREAMER_LIBRARIES}
	${GSTREAMER_PLUGIN_VIDEO_LIBRARY}
	CACHE INTERNAL EXPORTEDVARIABLE
)
set(videoplayer_gstreamer_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/gstreamerplayerbackend.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/gstreamerconfigwidget.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/gstreamer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/../../helpers/languagecode.cpp
	${videoplayerplugins_SRCS}
	CACHE INTERNAL EXPORTEDVARIABLE
)

ki18n_wrap_ui(videoplayer_gstreamer_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/gstreamerconfigwidget.ui
)

add_library(gstplayer MODULE ${videoplayer_gstreamer_SRCS})

install(TARGETS gstplayer DESTINATION ${SC_PLUGIN_INSTALL_DIR})

target_include_directories(gstplayer SYSTEM PRIVATE ${videoplayer_gstreamer_INCLUDE_DIR})
set_target_properties(gstplayer PROPERTIES COMPILE_FLAGS "${videoplayer_gstreamer_DEFS}")
target_link_libraries(gstplayer ${subtitlecomposer_LIBS} ${videoplayer_gstreamer_LIBS} Qt5::Core)

add_dependencies(gstplayer subtitlecomposer)
