set(core_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/formatdata.h
	${CMAKE_CURRENT_SOURCE_DIR}/range.h
	${CMAKE_CURRENT_SOURCE_DIR}/rangelist.h
	${CMAKE_CURRENT_SOURCE_DIR}/time.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/sstring.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/subtitle.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/subtitleiterator.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/subtitleline.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/undo/subtitleactions.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/undo/subtitlelineactions.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/undo/undoaction.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/undo/undostack.cpp
	CACHE INTERNAL EXPORTEDVARIABLE
)

add_subdirectory(tests)
