if(NOT CMAKE_BUILD_TYPE)
	message(STATUS "No build type selected, default to Release")
	set(CMAKE_BUILD_TYPE "Release")
endif()

if(CMAKE_COMPILER_IS_GNUCC)
	message(STATUS "GNU C compiler detected")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -O0 -g")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -O0 -g")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -O2 -pipe -fstack-protector --param=ssp-buffer-size=4")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -O2 -pipe -fstack-protector --param=ssp-buffer-size=4")
	set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELEASE} -g")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELEASE} -g")
endif()

include_directories(
	${CMAKE_CURRENT_BINARY_DIR} # so files can find config.h
	${CMAKE_CURRENT_SOURCE_DIR} # so that we can refer to our files starting at src dir
	${Qt5Core_INCLUDE_DIRS} ${Qt5Widgets_INCLUDE_DIRS} ${Qt5Gui_INCLUDE_DIRS}
)

add_definitions(${QT_DEFINITIONS} -fexceptions -DHAVE_CONFIG_H)
add_definitions(-D_DEFAULT_SOURCE)

find_package(FFmpeg REQUIRED)

# the subdirs must be added in a specific order
# if one dir uses code from another, its CMakeLists.txt will reference variables
# defined in the dependency directory that must be created before the former
# CMakeLists.txt is processed

set(SC_PLUGIN_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/${LIB_INSTALL_DIR}/subtitlecomposer CACHE INTERNAL EXPORTEDVARIABLE)
message(STATUS "Subtitle Composer plugin path: ${SC_PLUGIN_INSTALL_DIR}")

set(subtitlecomposer_LIBS
	Qt5::Core Qt5::Widgets Qt5::Gui
	KF5::CoreAddons KF5::WidgetsAddons KF5::TextWidgets KF5::Codecs
	KF5::SonnetCore KF5::SonnetUi
	KF5::KIOCore KF5::KIOFileWidgets KF5::KIOWidgets KF5::KIONTLM
	KF5::KrossCore KF5::KrossUi KF5::XmlGui KF5::I18n
	CACHE INTERNAL EXPORTEDVARIABLE
)

add_subdirectory(helpers)
add_subdirectory(core)
add_subdirectory(formats)
add_subdirectory(widgets)
add_subdirectory(videoplayer)
add_subdirectory(streamprocessor)
add_subdirectory(speechprocessor)

add_subdirectory(configs)
add_subdirectory(dialogs)
add_subdirectory(icons)
add_subdirectory(actions)
add_subdirectory(utils)
add_subdirectory(scripting)

include_directories(
	${helpers_INCLUDE_DIR}
	${core_INCLUDE_DIR}
	${formats_INCLUDE_DIR}
	${videoplayer_INCLUDE_DIR}
	${streamprocessor_INCLUDE_DIR}
	${speechprocessor_INCLUDE_DIR}
	${widgets_INCLUDE_DIR}
	${FFMPEG_INCLUDE_DIR}
)

set(subtitlecomposer_SRCS
	${helpers_SRCS}
	${core_SRCS}
	${formats_SRCS}
	${videoplayer_SRCS}
	${streamprocessor_SRCS}
	${speechprocessor_SRCS}
	${widgets_SRCS}
	${main_configs_SRCS}
	${main_dialogs_SRCS}
	${main_actions_SRCS}
	${main_utils_SRCS}
	${main_scripting_SRCS}
	${CMAKE_CURRENT_SOURCE_DIR}/application.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/currentlinewidget.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/lineswidget.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/mainwindow.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/playerwidget.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/waveformwidget.cpp
	CACHE INTERNAL EXPORTEDVARIABLE
)

kconfig_add_kcfg_files(subtitlecomposer_SRCS GENERATE_MOC ${CMAKE_CURRENT_SOURCE_DIR}/scconfig.kcfgc)

ki18n_wrap_ui(subtitlecomposer_SRCS
	${formats_UIS}
	${main_configs_UIS}
	${main_dialogs_UIS}
	${main_scripting_UIS})

add_executable(subtitlecomposer ${subtitlecomposer_SRCS})

qt5_use_modules(subtitlecomposer Core Widgets)

# Xine videoplayer plugin needs XInitThreads() call in main
# optional since Xine will not be built without X11
find_package(X11 QUIET)

target_link_libraries(subtitlecomposer
	${subtitlecomposer_LIBS}
	${helpers_LIBS}
	${core_LIBS}
	${formats_LIBS}
	${videoplayer_LIBS}
	${streamprocessor_LIBS}
	${speechprocessor_LIBS}
	${widgets_LIBS}
	${X11_LIBRARIES}
	${FFMPEG_LIBRARIES}
)

add_definitions(
	${helpers_DEFS}
	${core_DEFS}
	${formats_DEFS}
	${videoplayer_DEFS}
	${streamprocessor_DEFS}
	${speechprocessor_DEFS}
	${widgets_DEFS}
)

install(TARGETS subtitlecomposer DESTINATION ${BIN_INSTALL_DIR})

install(FILES subtitlecomposerrc DESTINATION ${CONFIG_INSTALL_DIR})
install(FILES subtitlecomposerui.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/subtitlecomposer)
install(FILES subtitlecomposer.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES subtitlecomposer.xml DESTINATION ${XDG_MIME_INSTALL_DIR})

add_subdirectory(videoplayerplugins)

add_subdirectory(speechplugins)

# do the configuration of config.h at the end, so all the necessary variables have been set
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

