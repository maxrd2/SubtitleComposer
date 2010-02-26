# Tries to find ICU library
# Once done this will define
#
#  ICU_FOUND - system has ICU library
#  ICU_LIBRARIES - Link these to use ICU library
#  ICU_INCLUDE_DIRS - the ICU library include dirs
#  ICU_DEFINITIONS - compiler switches required for using ICU library

IF ( ICU_INCLUDE_DIR AND ICU_LIBRARIES ) # in cache already
	SET( ICU_FIND_QUIETLY TRUE )
ENDIF ( ICU_INCLUDE_DIR AND ICU_LIBRARIES )

# EXECUTE_PROCESS(
# 	WORKING_DIRECTORY .
# 	COMMAND icu-config --cppflags-searchpath
# 	RESULT_VARIABLE ret_var
# 	OUTPUT_VARIABLE ICU_INCLUDE_DIR
# )

FIND_PATH(
	ICU_INCLUDE_DIR
	NAMES unicode/utypes.h
	DOC "Include directory for the ICU library"
)

MARK_AS_ADVANCED( ICU_INCLUDE_DIR )

# EXECUTE_PROCESS(
# 	WORKING_DIRECTORY .
# 	COMMAND icu-config --ldflags-libsonly
# 	RESULT_VARIABLE ret_var
# 	OUTPUT_VARIABLE ICU_LIBRARY
# )

# FIND_LIBRARY(
# 	ICU_LIBRARY
# #	NAMES icuuc cygicuuc cygicuuc32
# 	NAMES icuuc icui18n icudata
# 	DOC "Libraries to link against for the common parts of ICU"
# )

SET( ICU_LIBRARY -L/usr/lib -licui18n -licuuc -licudata )

MARK_AS_ADVANCED( ICU_LIBRARIES )

IF( ICU_INCLUDE_DIR AND ICU_LIBRARY )
	MESSAGE( STATUS "Found ICU library: ${ICU_INCLUDE_DIR}" )
	SET( ICU_FOUND 1 )
	SET( ICU_LIBRARIES ${ICU_LIBRARY} )
	SET( ICU_INCLUDE_DIRS ${ICU_INCLUDE_DIR} )
	SET( ICU_DEFINITIONS -D_REENTRANT )
ELSE( ICU_INCLUDE_DIR AND ICU_LIBRARY )
	SET( ICU_FOUND 0 )
	SET( ICU_LIBRARIES )
	SET( ICU_INCLUDE_DIRS )
	SET( ICU_DEFINITIONS )
ENDIF( ICU_INCLUDE_DIR AND ICU_LIBRARY )
