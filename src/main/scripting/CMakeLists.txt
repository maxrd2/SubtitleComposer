ADD_SUBDIRECTORY( examples )

SET( main_scripting_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/scripting_rangesmodule.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/scripting_stringsmodule.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/scripting_subtitlemodule.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/scripting_subtitlelinemodule.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/scripting_list.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/scripting_range.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/scripting_rangelist.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/scripting_sstring.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/scripting_subtitle.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/scripting_subtitleline.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/scriptsmanager.cpp
	CACHE INTERNAL EXPORTEDVARIABLE
)
