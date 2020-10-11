if(NOT CMAKE_BUILD_TYPE)
	message(STATUS "No build type selected, default to Release")
	set(CMAKE_BUILD_TYPE "Release")
endif()

if(CMAKE_COMPILER_IS_GNUCC)
	message(STATUS "GNU C compiler detected")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Og -g")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Og -g")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -O3")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -O3")
	set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELEASE} -Wall -O3 -g")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELEASE} -Wall -O3 -g")
endif()

ecm_setup_version(${PROJECT_VERSION}
	VARIABLE_PREFIX SUBTITLECOMPOSER
	SOVERSION ${PROJECT_VERSION_MAJOR}
	VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/config-subtitlecomposer.h"
)

add_definitions(${QT_DEFINITIONS} -DHAVE_CONFIG_H)
add_definitions(-D_DEFAULT_SOURCE)

find_package(FFmpeg 57.83.100 REQUIRED)
find_package(OpenAL REQUIRED)
find_package(ICU)

# the subdirs must be added in a specific order
# if one dir uses code from another, its CMakeLists.txt will reference variables
# defined in the dependency directory that must be created before the former
# CMakeLists.txt is processed

set(SC_PLUGIN_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/${KDE_INSTALL_LIBDIR}/subtitlecomposer" CACHE STRING EXPORTEDVARIABLE)
message(STATUS "Subtitle Composer plugin path: ${SC_PLUGIN_INSTALL_DIR}")

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR} # so that we can refer to our files starting at src dir
	${CMAKE_CURRENT_SOURCE_DIR}/widgets # needed for .ui files to access custom widgets
	${Qt5Core_INCLUDE_DIRS}
	${Qt5Widgets_INCLUDE_DIRS}
	${Qt5Gui_INCLUDE_DIRS}
	${OPENAL_INCLUDE_DIR}
	${FFMPEG_INCLUDE_DIRS}
)

add_subdirectory(helpers)
add_subdirectory(core)
add_subdirectory(gui)
add_subdirectory(formats)
add_subdirectory(widgets)
add_subdirectory(videoplayer)
add_subdirectory(streamprocessor)
add_subdirectory(speechprocessor)

add_subdirectory(configs)
add_subdirectory(dialogs)
add_subdirectory(errors)
add_subdirectory(actions)
add_subdirectory(utils)
add_subdirectory(scripting)

set(subtitlecomposer_SRCS
	${core_SRCS}
	${gui_SRCS}
	${dialogs_SRCS}
	${errors_SRCS}
	${formats_SRCS}
	${helpers_SRCS}
	${speechprocessor_SRCS}
	${streamprocessor_SRCS}
	${videoplayer_SRCS}
	${widgets_SRCS}
	${main_configs_SRCS}
	${main_actions_SRCS}
	${main_utils_SRCS}
	${main_scripting_SRCS}
	${CMAKE_CURRENT_SOURCE_DIR}/application.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/application_errorcheck.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/application_subtitle.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/mainwindow.cpp
	CACHE INTERNAL EXPORTEDVARIABLE
)

if(MINGW)
	qt5_add_resources(subtitlecomposer_SRCS subtitlecomposer.qrc)
	list(APPEND subtitlecomposer_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/subtitlecomposer.rc)
else()
	add_subdirectory(icons)
endif()

kconfig_add_kcfg_files(subtitlecomposer_SRCS GENERATE_MOC ${CMAKE_CURRENT_SOURCE_DIR}/scconfig.kcfgc)

ki18n_wrap_ui(subtitlecomposer_SRCS
	${dialogs_UIS}
	${formats_UIS}
	${main_configs_UIS}
	${main_scripting_UIS})

add_executable(subtitlecomposer WIN32 ${subtitlecomposer_SRCS})

if(ICU_FOUND)
	message(STATUS "ICU Library will be used for charset detection.")
	set(HAVE_ICU 1 CACHE INTERNAL EXPORTEDVARIABLE)
	include_directories(${ICU_INCLUDE_DIRS})
else()
	message(STATUS "ICU Library not found. KEncodingProber fallback will be used for charset detection.")
endif()

target_link_libraries(subtitlecomposer
	Qt5::Core Qt5::Widgets Qt5::Gui
	KF5::CoreAddons KF5::WidgetsAddons KF5::TextWidgets KF5::Codecs
	KF5::SonnetCore KF5::SonnetUi
	KF5::KIOCore KF5::KIOFileWidgets KF5::KIOWidgets KF5::KIONTLM
	KF5::KrossCore KF5::KrossUi KF5::XmlGui KF5::I18n
	${FFMPEG_LIBRARIES}
	${OPENAL_LIBRARY}
	${ICU_LIBRARIES}
)

install(TARGETS subtitlecomposer DESTINATION ${BIN_INSTALL_DIR})

install(FILES subtitlecomposerrc DESTINATION ${CONFIG_INSTALL_DIR})
install(FILES subtitlecomposerui.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/subtitlecomposer)
install(FILES org.kde.subtitlecomposer.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES org.kde.subtitlecomposer.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES subtitlecomposer.xml DESTINATION ${XDG_MIME_INSTALL_DIR})

add_subdirectory(speechplugins)

# do the configuration of config.h at the end, so all the necessary variables have been set
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

