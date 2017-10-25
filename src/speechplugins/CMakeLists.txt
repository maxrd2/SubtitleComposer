set(speechplugins_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/../speechprocessor/speechplugin.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/../scconfigdummy.cpp
	CACHE INTERNAL EXPORTEDVARIABLE
)

add_subdirectory(pocketsphinx)
