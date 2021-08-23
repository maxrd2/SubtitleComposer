# Tries to find ICU library
# Once done this will define
#
#  ICU_FOUND - System has ICU library
#  ICU_VERSION - ICU library version
#  ICU_LIBRARIES - Link these to use ICU library
#  ICU_INCLUDE_DIRS - ICU library include dirs
#  ICU_DEFINITIONS - compiler switches required for using ICU library

# SPDX-FileCopyrightText: 2010-2019 Mladen Milinkovic <maxrd2@smoothware.net>
# SPDX-License-Identifier: BSD-3-Clause

find_package(PkgConfig REQUIRED)

pkg_check_modules(PC_ICU QUIET icu-i18n)

find_path(ICU_INCLUDE_DIRS
	unicode/utypes.h
	HINTS ${PC_ICU_INCLUDEDIR} ${PC_ICU_INCLUDE_DIRS}
	DOC "Include directory for the ICU library")
set(_required_vars ICU_INCLUDE_DIRS)

foreach(_lib ${PC_ICU_LIBRARIES})
	find_library(_lib_${_lib}
		NAMES ${_lib}
		HINTS ${PC_ICU_LIBDIR} ${PC_ICU_LIBRARY_DIRS})
	list(APPEND ICU_LIBRARIES ${_lib_${_lib}})
	list(APPEND _required_vars _lib_${_lib})
endforeach()

set(_message "\n\tincludes: ${ICU_INCLUDE_DIRS}\n\tlibs: ${ICU_LIBRARIES}\n\t")

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set FFMPEG_FOUND to TRUE if all listed variables are TRUE
find_package_handle_standard_args(ICU
	REQUIRED_VARS _message ${_required_vars}
	VERSION_VAR PC_ICU_VERSION)

if(ICU_FOUND)
	set(ICU_DEFINITIONS -D_REENTRANT)
endif()

mark_as_advanced(ICU_INCLUDE_DIRS ICU_LIBRARIES ICU_DEFINITIONS)
