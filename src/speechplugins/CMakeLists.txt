set(speechplugins_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/../speechprocessor/speechplugin.cpp
	CACHE INTERNAL EXPORTEDVARIABLE
)

add_subdirectory(pocketsphinx)
