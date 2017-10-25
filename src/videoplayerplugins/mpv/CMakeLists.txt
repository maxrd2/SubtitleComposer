find_package(MPV 1.14)
if(NOT MPV_FOUND)
	message(STATUS "Have NOT Found LibMPV v1.14 - MPV videoplayer plugin will not be built")
	return()
endif(NOT MPV_FOUND)

set(HAVE_MPV 1 CACHE INTERNAL EXPORTEDVARIABLE)

set(videoplayer_mpv_INCLUDE_DIR
	${MPV_INCLUDE_DIRS}
	CACHE INTERNAL EXPORTEDVARIABLE
)
set(videoplayer_mpv_DEFS
	${MPV_DEFINITIONS}
	CACHE INTERNAL EXPORTEDVARIABLE
)
set(videoplayer_mpv_LIBS
	${MPV_LIBRARIES}
	CACHE INTERNAL EXPORTEDVARIABLE
)
set(videoplayer_mpv_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/mpvbackend.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/mpvconfigwidget.cpp
	${videoplayerplugins_SRCS}
	CACHE INTERNAL EXPORTEDVARIABLE
)

ki18n_wrap_ui(videoplayer_mpv_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/mpvconfigwidget.ui
)

add_library(mpvplayer MODULE ${videoplayer_mpv_SRCS})

install(TARGETS mpvplayer DESTINATION ${SC_PLUGIN_INSTALL_DIR})

target_include_directories(mpvplayer SYSTEM PRIVATE ${videoplayer_mpv_INCLUDE_DIR})
set_target_properties(mpvplayer PROPERTIES COMPILE_FLAGS "${videoplayer_mpv_DEFS}")
target_link_libraries(mpvplayer ${subtitlecomposer_LIBS} ${videoplayer_mpv_LIBS})
qt5_use_modules(mpvplayer Core)

add_dependencies(mpvplayer subtitlecomposer)
