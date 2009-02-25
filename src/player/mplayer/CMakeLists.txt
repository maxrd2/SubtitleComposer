SET( players_mplayer_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/mediadata.h
	${CMAKE_CURRENT_SOURCE_DIR}/mplayerbackend.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/mplayerconfigwidget.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/mplayerprocess.cpp
	CACHE INTERNAL EXPORTEDVARIABLE
)

INSTALL( FILES input.conf DESTINATION ${DATA_INSTALL_DIR}/subtitlecomposer )
