MACRO_OPTIONAL_FIND_PACKAGE( GStreamer )
IF( GSTREAMER_FOUND )
	SET( HAVE_GSTREAMER 1 CACHE INTERNAL EXPORTEDVARIABLE )
	ADD_SUBDIRECTORY( gstreamer )
ELSE( GSTREAMER_FOUND )
	MESSAGE( STATUS "GStreamer was not found. GStreamer backend will not be built" )
ENDIF( GSTREAMER_FOUND )

ADD_SUBDIRECTORY( mplayer )
ADD_SUBDIRECTORY( phonon )

MACRO_OPTIONAL_FIND_PACKAGE( Xine )
IF( XINE_FOUND )
	SET( HAVE_XINE 1 CACHE INTERNAL EXPORTEDVARIABLE )
	ADD_SUBDIRECTORY( xine )
ELSE( XINE_FOUND )
	MESSAGE( STATUS "Xine was not found. Xine backend will not be built" )
ENDIF( XINE_FOUND )

SET( players_INCLUDE_DIR
	${players_gstreamer_INCLUDE_DIR}
	${players_mplayer_INCLUDE_DIR}
	${players_phonon_INCLUDE_DIR}
	${players_xine_INCLUDE_DIR}
	CACHE INTERNAL EXPORTEDVARIABLE
)

SET( players_DEFS
	${players_gstreamer_DEFS}
	${players_mplayer_DEFS}
	${players_phonon_DEFS}
	${players_xine_DEFS}
	CACHE INTERNAL EXPORTEDVARIABLE
)

SET( players_LIBS
	${players_gstreamer_LIBS}
	${players_mplayer_LIBS}
	${players_phonon_LIBS}
	${players_xine_LIBS}
	CACHE INTERNAL EXPORTEDVARIABLE
)

SET( players_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/player.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/playerbackend.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/videowidget.cpp
	${players_gstreamer_SRCS}
	${players_mplayer_SRCS}
	${players_phonon_SRCS}
	${players_xine_SRCS}
	CACHE INTERNAL EXPORTEDVARIABLE
)
