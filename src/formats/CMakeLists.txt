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
add_subdirectory(vobsub)

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
	${formats_textdemux_SRCS}
	${formats_tmplayer_SRCS}
	${formats_youtubecaptions_SRCS}
	${formats_textdemux_SRCS}
	${formats_vobsub_SRCS}
	CACHE INTERNAL EXPORTEDVARIABLE
)

set(formats_UIS
	${formats_vobsub_UIS}
	CACHE INTERNAL EXPORTEDVARIABLE
)
