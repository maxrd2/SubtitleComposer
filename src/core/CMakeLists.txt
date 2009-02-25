SET( core_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/action.h
	${CMAKE_CURRENT_SOURCE_DIR}/compositeaction.h
	${CMAKE_CURRENT_SOURCE_DIR}/actionmanager.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/audiolevels.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/formatdata.h
	${CMAKE_CURRENT_SOURCE_DIR}/range.h
	${CMAKE_CURRENT_SOURCE_DIR}/rangelist.h
	${CMAKE_CURRENT_SOURCE_DIR}/time.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/sstring.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/subtitle.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/subtitleactions.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/subtitleiterator.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/subtitleline.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/subtitlelineactions.cpp
	CACHE INTERNAL EXPORTEDVARIABLE
)

ADD_SUBDIRECTORY( tests )
