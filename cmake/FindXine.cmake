# - Try to find the XINE  library
# Once done this will define
#
#  XINE_FOUND - system has the XINE library
#  XINE_VERSION - XINE version
#  XINE_BUGFIX_VERSION - the XINE bugfix version
#  XINE_INCLUDE_DIR - the XINE include directory
#  XINE_LIBRARY - The libraries needed to use XINE
#  XINE_XCB_FOUND - libxine can use XCB for video output

# Copyright (c) 2008 Helio Chissini de Castro, <helio@kde.org>
# Copyright (c) 2006,2007 Laurent Montel, <montel@kde.org>
# Copyright (c) 2006, Matthias Kretz, <kretz@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(XINE_INCLUDE_DIR AND XINE_LIBRARY)
	# Already in cache, be silent
	set(Xine_FIND_QUIETLY TRUE)
endif(XINE_INCLUDE_DIR AND XINE_LIBRARY)

IF (NOT WIN32)
	FIND_PACKAGE(PkgConfig)
	PKG_CHECK_MODULES(PKG_XINE libxine)
ENDIF (NOT WIN32)

FIND_PATH(XINE_INCLUDE_DIR NAMES xine.h
	PATHS ${PKG_XINE_INCLUDE_DIRS} )

FIND_LIBRARY(XINE_LIBRARY NAMES xine
	PATHS ${PKG_XINE_LIBRARY_DIRS} )

if(XINE_INCLUDE_DIR AND XINE_LIBRARY)
	if(NOT PKG_XINE_VERSION VERSION_LESS 1.1.9)
		set(XINE_FOUND TRUE)
		string(REGEX REPLACE "[0-9].[0-9]." "" XINE_BUGFIX_VERSION ${PKG_XINE_VERSION})
		set(XINE_VERSION ${PKG_XINE_VERSION})
	endif(NOT PKG_XINE_VERSION VERSION_LESS 1.1.9)
endif(XINE_INCLUDE_DIR AND XINE_LIBRARY)


if(XINE_FOUND)
	INCLUDE(CheckCSourceCompiles)
	SET(CMAKE_REQUIRED_INCLUDES ${XINE_INCLUDE_DIR})
	SET(CMAKE_REQUIRED_LIBRARIES ${XINE_LIBRARY})
	CHECK_C_SOURCE_COMPILES("#include <xine.h>\nint main()\n{\n  xine_open_video_driver(xine_new(), \"auto\", XINE_VISUAL_TYPE_XCB, NULL);\n  return 0;\n}\n" XINE_XCB_FOUND)
endif(XINE_FOUND)

if(XINE_FOUND)
	if(NOT Xine_FIND_QUIETLY)
		message(STATUS "Found XINE: ${XINE_LIBRARY}")
	endif(NOT Xine_FIND_QUIETLY)
endif(XINE_FOUND)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Xine "Could NOT find XINE 1.1.9 or greater"  XINE_INCLUDE_DIR XINE_LIBRARY)

MARK_AS_ADVANCED(XINE_INCLUDE_DIR XINE_LIBRARY)
