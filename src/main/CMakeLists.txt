add_subdirectory(configs)
add_subdirectory(dialogs)
add_subdirectory(icons)
add_subdirectory(actions)
add_subdirectory(utils)
add_subdirectory(scripting)

include_directories(
	${common_INCLUDE_DIR}
	${core_INCLUDE_DIR}
	${formats_INCLUDE_DIR}
	${videoplayer_INCLUDE_DIR}
	${streamprocessor_INCLUDE_DIR}
	${speechprocessor_INCLUDE_DIR}
	${widgets_INCLUDE_DIR}
)

set(subtitlecomposer_SRCS
	${common_SRCS}
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
	${CMAKE_CURRENT_SOURCE_DIR}/errorsdialog.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/errorswidget.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/lineswidget.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/mainwindow.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/playerwidget.cpp
	CACHE INTERNAL EXPORTEDVARIABLE
)

kconfig_add_kcfg_files(subtitlecomposer_SRCS GENERATE_MOC ${CMAKE_CURRENT_SOURCE_DIR}/scconfig.kcfgc)

ki18n_wrap_ui(subtitlecomposer_SRCS
	${main_configs_UIS}
	${main_dialogs_UIS}
	${main_scripting_UIS})

add_executable(subtitlecomposer ${subtitlecomposer_SRCS})

qt5_use_modules(subtitlecomposer Core Widgets)

# Xine videoplayer plugin needs XInitThreads() call in main
# optional since Xine will not be built without X11
find_package(X11 QUIET)

target_link_libraries(subtitlecomposer
	${common_LIBS}
	${core_LIBS}
	${formats_LIBS}
	${videoplayer_LIBS}
	${streamprocessor_LIBS}
	${speechprocessor_LIBS}
	${widgets_LIBS}
	${X11_LIBRARIES}
)

add_definitions(
	${common_DEFS}
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
