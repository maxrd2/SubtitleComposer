set(speechplugins_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/../speechprocessor/speechplugin.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/scconfigdummy.cpp
	CACHE INTERNAL EXPORTEDVARIABLE
)

find_package(PocketSphinx QUIET)
if(POCKETSPHINX_FOUND)
	message(STATUS "Found PocketSphinx version ${POCKETSPHINX_VERSION}")
	add_subdirectory(pocketsphinx)
else(POCKETSPHINX_FOUND)
	message(WARNING "PocketSphinx not found - Speech plugin will not be built")
endif(POCKETSPHINX_FOUND)
