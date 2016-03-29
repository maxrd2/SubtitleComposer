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
	${CMAKE_CURRENT_SOURCE_DIR}/xineconfigwidget.ui
	${videoplayerplugins_SRCS}
	CACHE INTERNAL EXPORTEDVARIABLE
)

find_package(XCB)
IF(XCB_FOUND AND XINE_XCB_FOUND)
	set(HAVE_XCB 1 CACHE INTERNAL EXPORTEDVARIABLE)
	set(videoplayer_xine_INCLUDE_DIR
		${LIBXCB_INCLUDE_DIR}
		${videoplayer_xine_INCLUDE_DIR}
		CACHE INTERNAL EXPORTEDVARIABLE)
	set(videoplayer_xine_DEFS
		${LIBXCB_DEFINITIONS}
		${videoplayer_xine_DEFS}
		CACHE INTERNAL EXPORTEDVARIABLE)
	set(videoplayer_xine_LIBS
		${LIBXCB_LIBRARIES}
		${videoplayer_xine_LIBS}
		CACHE INTERNAL EXPORTEDVARIABLE)
endif(XCB_FOUND AND XINE_XCB_FOUND)

add_library(xineplayer MODULE ${videoplayer_xine_SRCS})

install(TARGETS xineplayer DESTINATION ${SC_PLUGIN_INSTALL_DIR})

target_include_directories(xineplayer SYSTEM PRIVATE ${videoplayer_xine_INCLUDE_DIR})
set_target_properties(xineplayer PROPERTIES COMPILE_FLAGS "${videoplayer_xine_DEFS}")
target_link_libraries(xineplayer ${common_LIBS} ${videoplayer_xine_LIBS})
qt5_use_modules(xineplayer Core)

add_dependencies(xineplayer subtitlecomposer)
