find_package(Xine 1.1.9)
find_package(X11)
if(NOT XINE_FOUND OR NOT X11_FOUND)
	message(STATUS "Have NOT found Xine v1.1.9+ and/or LibX11 - Xine videoplayer plugin will not be built")
	return()
endif(NOT XINE_FOUND OR NOT X11_FOUND)

set(HAVE_XINE 1 CACHE INTERNAL EXPORTEDVARIABLE)

set(videoplayer_xine_INCLUDE_DIR
	${XINE_INCLUDE_DIR}
	CACHE INTERNAL EXPORTEDVARIABLE
)
set(videoplayer_xine_DEFS
	-D_FILE_OFFSET_BITS=64
	CACHE INTERNAL EXPORTEDVARIABLE
)
set(videoplayer_xine_LIBS
	${XINE_LIBRARY}
	${X11_LIBRARIES}
	CACHE INTERNAL EXPORTEDVARIABLE
)
set(videoplayer_xine_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/xineplayerbackend.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/xinevideolayerwidget.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/xineconfigwidget.cpp
	${videoplayerplugins_SRCS}
	CACHE INTERNAL EXPORTEDVARIABLE
)

ki18n_wrap_ui(videoplayer_xine_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/xineconfigwidget.ui
)

find_package(XCB QUIET COMPONENTS XCB)
IF(XCB_FOUND AND XINE_XCB_FOUND)
	message(STATUS "Found XCB: ${XCB_LIBRARIES} (version ${XCB_VERSION})")
	message(STATUS "Xine Player backend will use XCB instead of LibX11")
	set(HAVE_XCB 1 CACHE INTERNAL EXPORTEDVARIABLE)
	set(videoplayer_xine_INCLUDE_DIR
		${XCB_INCLUDE_DIR}
		${videoplayer_xine_INCLUDE_DIR}
		CACHE INTERNAL EXPORTEDVARIABLE)
	set(videoplayer_xine_DEFS
		${XCB_DEFINITIONS}
		${videoplayer_xine_DEFS}
		CACHE INTERNAL EXPORTEDVARIABLE)
	set(videoplayer_xine_LIBS
		${XCB_LIBRARIES}
		${videoplayer_xine_LIBS}
		CACHE INTERNAL EXPORTEDVARIABLE)
else(XCB_FOUND AND XINE_XCB_FOUND)
	message(STATUS "Xine XCB support not available (Found XCB version ${XCB_VERSION})")
endif(XCB_FOUND AND XINE_XCB_FOUND)

add_library(xineplayer MODULE ${videoplayer_xine_SRCS})

install(TARGETS xineplayer DESTINATION ${SC_PLUGIN_INSTALL_DIR})

target_include_directories(xineplayer SYSTEM PRIVATE ${videoplayer_xine_INCLUDE_DIR})
set_target_properties(xineplayer PROPERTIES COMPILE_FLAGS "${videoplayer_xine_DEFS}")
target_link_libraries(xineplayer ${subtitlecomposer_LIBS} ${videoplayer_xine_LIBS})
qt5_use_modules(xineplayer Core)

add_dependencies(xineplayer subtitlecomposer)
