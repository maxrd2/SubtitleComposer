set(videoplayer_mplayer_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/mediadata.h
	${CMAKE_CURRENT_SOURCE_DIR}/mplayerplayerbackend.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/mplayerplayerprocess.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/mplayerconfigwidget.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/mplayerconfigwidget.ui
	${CMAKE_CURRENT_SOURCE_DIR}/../../common/qxtsignalwaiter.cpp
	${videoplayerplugins_SRCS}
	CACHE INTERNAL EXPORTEDVARIABLE
)

add_library(mplayer MODULE ${videoplayer_mplayer_SRCS})

install(TARGETS mplayer DESTINATION ${VIDEOPLAYERPLUGIN_INSTALL_DIR})

target_include_directories(mplayer SYSTEM PRIVATE ${videoplayer_mplayer_INCLUDE_DIR})
set_target_properties(mplayer PROPERTIES COMPILE_FLAGS "${videoplayer_mplayer_DEFS}")
target_link_libraries(mplayer ${common_LIBS} ${videoplayer_mplayer_LIBS})

qt5_use_modules(mplayer Core)

add_dependencies(mplayer subtitlecomposer)
