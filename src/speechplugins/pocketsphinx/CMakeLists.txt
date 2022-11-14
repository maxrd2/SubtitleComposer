find_package(PocketSphinx 5)
if(NOT POCKETSPHINX_FOUND)
	message(STATUS "Have NOT Found PocketSphinx - Speech plugin will not be built")
	return()
endif()

set(speech_pocketsphinx_SRCS
	../../speechprocessor/speechplugin.cpp
	pocketsphinxplugin.cpp
	pocketsphinxconfigwidget.cpp
	CACHE INTERNAL EXPORTEDVARIABLE
)
ki18n_wrap_ui(speech_pocketsphinx_SRCS
	pocketsphinxconfigwidget.ui
)
kconfig_add_kcfg_files(speech_pocketsphinx_SRCS GENERATE_MOC
	${CMAKE_CURRENT_SOURCE_DIR}/pocketsphinxconfig.kcfgc
)

add_library(pocketsphinxasr MODULE ${speech_pocketsphinx_SRCS})

install(TARGETS pocketsphinxasr DESTINATION ${SC_PLUGIN_INSTALL_DIR})

target_include_directories(pocketsphinxasr SYSTEM PRIVATE
	${CMAKE_CURRENT_BINARY_DIR} # so files can find plugin-config.h
	${POCKETSPHINX_INCLUDE_DIRS})
target_link_libraries(pocketsphinxasr
	Qt${QT_MAJOR_VERSION}::Core Qt${QT_MAJOR_VERSION}::Widgets Qt${QT_MAJOR_VERSION}::Gui
	KF5::I18n
	KF5::KIOCore KF5::KIOFileWidgets KF5::KIOWidgets KF5::KIONTLM
	${POCKETSPHINX_LIBRARIES})

add_dependencies(pocketsphinxasr subtitlecomposer)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/plugin-config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/plugin-config.h)

set(CMAKE_REQUIRED_INCLUDES ${POCKETSPHINX_INCLUDE_DIRS})
set(CMAKE_REQUIRED_LIBRARIES ${POCKETSPHINX_LIBRARIES})
check_cxx_source_compiles("#include <pocketsphinx.h>\nint main(){ ps_seg_iter(nullptr); return 0; }" PocketSphinx_NEW_ps_seg_iter)
if(PocketSphinx_NEW_ps_seg_iter)
	target_compile_definitions(pocketsphinxasr PRIVATE HAS_NEW_PS_SEG_ITER=1)
endif()
