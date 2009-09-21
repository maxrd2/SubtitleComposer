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

SET( services_INCLUDE_DIR
	${services_gstreamer_INCLUDE_DIR}
	${services_mplayer_INCLUDE_DIR}
	${services_phonon_INCLUDE_DIR}
	${services_xine_INCLUDE_DIR}
	CACHE INTERNAL EXPORTEDVARIABLE
)

SET( services_DEFS
	${services_gstreamer_DEFS}
	${services_mplayer_DEFS}
	${services_phonon_DEFS}
	${services_xine_DEFS}
	CACHE INTERNAL EXPORTEDVARIABLE
)

SET( services_LIBS
	${services_gstreamer_LIBS}
	${services_mplayer_LIBS}
	${services_phonon_LIBS}
	${services_xine_LIBS}
	CACHE INTERNAL EXPORTEDVARIABLE
)

SET( services_SRCS
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

ADD_SUBDIRECTORY( tests )
