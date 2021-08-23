# Tries to find FFmpeg libraries
# Once done this will define
#
#  FFMPEG_FOUND - System has FFmpeg
#  FFMPEG_VERSION - FFmpeg version (it's actually libavformat version)
#  FFMPEG_INCLUDE_DIRS - FFmpeg include directories
#  FFMPEG_LIBRARIES - FFmpeg libraries
#
# disabled: AVDEVICE POSTPROC AVFILTER
set(_avmodules AVFORMAT AVCODEC SWRESAMPLE SWSCALE AVUTIL)
#  FFMPEG_(avmodule)_VERSION - module version
#  FFMPEG_(avmodule)_INCLUDE_DIR - module include directory
#  FFMPEG_(avmodule)_LIBRARY - module library

# SPDX-FileCopyrightText: 2010-2019 Mladen Milinkovic <maxrd2@smoothware.net>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

find_package(PkgConfig REQUIRED)

foreach(MODULE ${_avmodules})
	string(TOLOWER ${MODULE} module)

	pkg_check_modules(PC_FFMPEG_${MODULE} QUIET lib${module})
	set(FFMPEG_${MODULE}_VERSION ${PC_FFMPEG_${MODULE}_VERSION})

	find_path(FFMPEG_${MODULE}_INCLUDE_DIR lib${module}/version.h
		HINTS
			${PC_FFMPEG_${MODULE}_INCLUDEDIR} ${PC_FFMPEG_${MODULE}_INCLUDE_DIRS}
		PATH_SUFFIXES ffmpeg)
	list(APPEND _required_vars FFMPEG_${MODULE}_INCLUDE_DIR)
	list(APPEND FFMPEG_INCLUDE_DIRS ${FFMPEG_${MODULE}_INCLUDE_DIR})

	find_library(FFMPEG_${MODULE}_LIBRARY
		NAMES ${module}
		HINTS
			${PC_FFMPEG_${MODULE}_LIBDIR} ${PC_FFMPEG_${MODULE}_LIBRARY_DIRS})
	list(APPEND _required_vars FFMPEG_${MODULE}_LIBRARY)
	list(APPEND FFMPEG_LIBRARIES ${FFMPEG_${MODULE}_LIBRARY})
endforeach()
list(REMOVE_DUPLICATES FFMPEG_INCLUDE_DIRS)

set(_message "\n\tincludes: ${FFMPEG_INCLUDE_DIRS}\n\tlibs:")
foreach(MODULE ${_avmodules})
	set(_message "${_message}\t${FFMPEG_${MODULE}_LIBRARY} (version ${FFMPEG_${MODULE}_VERSION})\n\t")
endforeach()

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set FFMPEG_FOUND to TRUE if all listed variables are TRUE
find_package_handle_standard_args(FFmpeg
	REQUIRED_VARS _message ${_required_vars}
	VERSION_VAR FFMPEG_AVFORMAT_VERSION)

#if(FFMPEG_FOUND AND NOT FFmpeg_FIND_QUIETLY)
#	message(STATUS "Found FFmpeg:\n\tinclude dir: ${FFMPEG_INCLUDE_DIRS}")
#	foreach(MODULE ${_avmodules})
#		message(STATUS "\t${FFMPEG_${MODULE}_LIBRARY} (version ${FFMPEG_${MODULE}_VERSION})")
#	endforeach()
#endif()

mark_as_advanced(FFMPEG_INCLUDE_DIRS FFMPEG_LIBRARIES ${_required_vars})
