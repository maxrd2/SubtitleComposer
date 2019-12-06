find_package(PocketSphinx 5)
if(NOT POCKETSPHINX_FOUND)
	message(STATUS "Have NOT Found PocketSphinx - Speech plugin will not be built")
	return()
endif()

set(speech_pocketsphinx_INCLUDE_DIRS
	${CMAKE_CURRENT_BINARY_DIR} # so files can find plugin-config.h
	${POCKETSPHINX_INCLUDE_DIRS}
	CACHE INTERNAL EXPORTEDVARIABLE
)
set(speech_pocketsphinx_DEFS
	${POCKETSPHINX_DEFINITIONS}
	CACHE INTERNAL EXPORTEDVARIABLE
)
set(speech_pocketsphinx_LIBS
	${POCKETSPHINX_LIBRARIES}
	CACHE INTERNAL EXPORTEDVARIABLE
)
set(speech_pocketsphinx_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/pocketsphinxplugin.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/pocketsphinxconfigwidget.cpp
	${speechplugins_SRCS}
	CACHE INTERNAL EXPORTEDVARIABLE
)
ki18n_wrap_ui(speech_pocketsphinx_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/pocketsphinxconfigwidget.ui
)
kconfig_add_kcfg_files(speech_pocketsphinx_SRCS GENERATE_MOC
	${CMAKE_CURRENT_SOURCE_DIR}/pocketsphinxconfig.kcfgc
)


add_library(pocketsphinxasr MODULE ${speech_pocketsphinx_SRCS})

install(TARGETS pocketsphinxasr DESTINATION ${SC_PLUGIN_INSTALL_DIR})

target_include_directories(pocketsphinxasr SYSTEM PRIVATE ${speech_pocketsphinx_INCLUDE_DIRS})
set_target_properties(pocketsphinxasr PROPERTIES COMPILE_FLAGS "${speech_pocketsphinx_DEFS}")
target_link_libraries(pocketsphinxasr ${subtitlecomposer_LIBS} ${speech_pocketsphinx_LIBS} Qt5::Core)

add_dependencies(pocketsphinxasr subtitlecomposer)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/plugin-config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/plugin-config.h)

