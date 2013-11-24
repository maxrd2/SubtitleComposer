# each format appends its files to formats_SRCS
ADD_SUBDIRECTORY( microdvd )
ADD_SUBDIRECTORY( mplayer )
ADD_SUBDIRECTORY( mplayer2 )
ADD_SUBDIRECTORY( subrip )
ADD_SUBDIRECTORY( substationalpha )
ADD_SUBDIRECTORY( subviewer1 )
ADD_SUBDIRECTORY( subviewer2 )
ADD_SUBDIRECTORY( tmplayer )
ADD_SUBDIRECTORY( youtubecaptions )

SET( formats_DEFS
	${formats_microdvd_DEFS}
	${formats_mplayer_DEFS}
	${formats_mplayer2_DEFS}
	${formats_subrip_DEFS}
	${formats_substationalpha_DEFS}
	${formats_subviewer1_DEFS}
	${formats_subviewer2_DEFS}
	${formats_tmplayer_DEFS}
	${formats_youtubecaptions_DEFS}
	CACHE INTERNAL EXPORTEDVARIABLE
)

SET( formats_LIBS
	${formats_microdvd_LIBS}
	${formats_mplayer_LIBS}
	${formats_mplayer2_LIBS}
	${formats_subrip_LIBS}
	${formats_substationalpha_LIBS}
	${formats_subviewer1_LIBS}
	${formats_subviewer2_LIBS}
	${formats_tmplayer_LIBS}
	${formats_youtubecaptions_LIBS}
	CACHE INTERNAL EXPORTEDVARIABLE
)

SET( formats_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/format.h
	${CMAKE_CURRENT_SOURCE_DIR}/formatmanager.h
	${CMAKE_CURRENT_SOURCE_DIR}/inputformat.h
	${CMAKE_CURRENT_SOURCE_DIR}/outputformat.h
	${CMAKE_CURRENT_SOURCE_DIR}/formatmanager.cpp
	${formats_microdvd_SRCS}
	${formats_mplayer_SRCS}
	${formats_mplayer2_SRCS}
	${formats_subrip_SRCS}
	${formats_substationalpha_SRCS}
	${formats_subviewer1_SRCS}
	${formats_subviewer2_SRCS}
	${formats_tmplayer_SRCS}
	${formats_youtubecaptions_SRCS}
	CACHE INTERNAL EXPORTEDVARIABLE
)

SET( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR} )
MACRO_OPTIONAL_FIND_PACKAGE( ICU )

IF( ICU_FOUND )
	MESSAGE( STATUS "FOUND FOUND" )

	SET( HAVE_ICU 1 CACHE INTERNAL EXPORTEDVARIABLE )

	INCLUDE_DIRECTORIES( ${ICU_INCLUDE_DIRS} )

	SET( formats_DEFS
		${ICU_DEFINITIONS}
		${formats_DEFS}
		CACHE INTERNAL EXPORTEDVARIABLE
	)
	SET( formats_LIBS
		${ICU_LIBRARIES}
		${formats_LIBS}
		CACHE INTERNAL EXPORTEDVARIABLE
	)

ENDIF( ICU_FOUND )
