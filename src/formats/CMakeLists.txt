# each format appends its files to formats_SRCS
add_subdirectory(microdvd)
add_subdirectory(mplayer)
add_subdirectory(mplayer2)
add_subdirectory(subrip)
add_subdirectory(substationalpha)
add_subdirectory(subviewer1)
add_subdirectory(subviewer2)
add_subdirectory(tmplayer)
add_subdirectory(youtubecaptions)
add_subdirectory(textdemux)

set(formats_DEFS
	${formats_microdvd_DEFS}
	${formats_mplayer_DEFS}
	${formats_mplayer2_DEFS}
	${formats_subrip_DEFS}
	${formats_substationalpha_DEFS}
	${formats_subviewer1_DEFS}
	${formats_subviewer2_DEFS}
	${formats_tmplayer_DEFS}
	${formats_youtubecaptions_DEFS}
	${formats_textdemux_DEFS}
	CACHE INTERNAL EXPORTEDVARIABLE
)

set(formats_LIBS
	${formats_microdvd_LIBS}
	${formats_mplayer_LIBS}
	${formats_mplayer2_LIBS}
	${formats_subrip_LIBS}
	${formats_substationalpha_LIBS}
	${formats_subviewer1_LIBS}
	${formats_subviewer2_LIBS}
	${formats_tmplayer_LIBS}
	${formats_youtubecaptions_LIBS}
	${formats_textdemux_LIBS}
	CACHE INTERNAL EXPORTEDVARIABLE
)

set(formats_SRCS
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
	${formats_textdemux_SRCS}
	CACHE INTERNAL EXPORTEDVARIABLE
)

find_package(ICU)
if(ICU_FOUND)
	message(STATUS "FOUND FOUND")

	set(HAVE_ICU 1 CACHE INTERNAL EXPORTEDVARIABLE)

	include_directories(${ICU_INCLUDE_DIRS})

	set(formats_DEFS
		${ICU_DEFINITIONS}
		${formats_DEFS}
		CACHE INTERNAL EXPORTEDVARIABLE)
	set(formats_LIBS
		${ICU_LIBRARIES}
		${formats_LIBS}
		CACHE INTERNAL EXPORTEDVARIABLE)

endif(ICU_FOUND)
