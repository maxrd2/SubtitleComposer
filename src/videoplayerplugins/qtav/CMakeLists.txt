find_package(QtAV)
if(NOT QTAV_FOUND)
	message(STATUS "Have NOT Found QtAV - QtAV videoplayer plugin will not be built")
	return()
endif()

set(videoplayer_qtav_INCLUDE_DIR
	${QTAV_INCLUDE_DIRS}
	${QTAVWIDGETS_INCLUDE_DIRS}
	CACHE INTERNAL EXPORTEDVARIABLE
)
set(videoplayer_qtav_LIBS
	${QTAV_LIBRARIES}
	${QTAVWIDGETS_LIBRARIES}
	CACHE INTERNAL EXPORTEDVARIABLE
)
set(videoplayer_qtav_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/qtavbackend.cpp
	${videoplayerplugins_SRCS}
	CACHE INTERNAL EXPORTEDVARIABLE
)

add_library(qtavplayer MODULE ${videoplayer_qtav_SRCS})

install(TARGETS qtavplayer DESTINATION ${SC_PLUGIN_INSTALL_DIR})

target_include_directories(qtavplayer SYSTEM PRIVATE ${videoplayer_qtav_INCLUDE_DIR})
target_link_libraries(qtavplayer ${subtitlecomposer_LIBS} ${videoplayer_qtav_LIBS} Qt5::Core)

add_dependencies(qtavplayer subtitlecomposer)
