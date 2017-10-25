find_package(Phonon4Qt5 4.6.60 NO_MODULE)
if(NOT Phonon4Qt5_FOUND)
	message(STATUS "Have NOT Found Phonon4Qt5 v4.6.60+ - Phonon videoplayer plugin will not be built")
	return()
endif(NOT Phonon4Qt5_FOUND)

set(videoplayer_phonon_INCLUDE_DIR
	${PHONON_INCLUDES}
	CACHE INTERNAL EXPORTEDVARIABLE
)
set(videoplayer_phonon_LIBS
	${PHONON_LIBRARIES}
	CACHE INTERNAL EXPORTEDVARIABLE
)
set(videoplayer_phonon_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/phononplayerbackend.cpp
	${videoplayerplugins_SRCS}
	CACHE INTERNAL EXPORTEDVARIABLE
)

add_library(phononplayer MODULE ${videoplayer_phonon_SRCS})

install(TARGETS phononplayer DESTINATION ${SC_PLUGIN_INSTALL_DIR})

target_include_directories(phononplayer SYSTEM PRIVATE ${videoplayer_phonon_INCLUDE_DIR})
set_target_properties(phononplayer PROPERTIES COMPILE_FLAGS "${videoplayer_phonon_DEFS}")
target_link_libraries(phononplayer ${subtitlecomposer_LIBS} ${videoplayer_phonon_LIBS})
qt5_use_modules(phononplayer Core)

add_dependencies(phononplayer subtitlecomposer)
