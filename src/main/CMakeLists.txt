add_subdirectory(configs)
add_subdirectory(dialogs)
add_subdirectory(images)
add_subdirectory(actions)
add_subdirectory(utils)
add_subdirectory(scripting)

include_directories(
	${common_INCLUDE_DIR}
	${core_INCLUDE_DIR}
	${formats_INCLUDE_DIR}
	${services_INCLUDE_DIR}
	${widgets_INCLUDE_DIR}
)

set(subtitlecomposer_SRCS
	${common_SRCS}
	${core_SRCS}
	${formats_SRCS}
	${services_SRCS}
	${widgets_SRCS}
	${main_configs_SRCS}
	${main_dialogs_SRCS}
	${main_actions_SRCS}
	${main_utils_SRCS}
	${main_scripting_SRCS}
	${CMAKE_CURRENT_SOURCE_DIR}/application.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/audiolevelswidget.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/currentlinewidget.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/errorsdialog.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/errorswidget.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/lineswidget.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/statusbar.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/mainwindow.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/playerwidget.cpp
	CACHE INTERNAL EXPORTEDVARIABLE
)

kconfig_add_kcfg_files(subtitlecomposer_SRCS GENERATE_MOC ${CMAKE_CURRENT_SOURCE_DIR}/scconfig.kcfgc)

add_executable(subtitlecomposer ${subtitlecomposer_SRCS})

qt5_use_modules(subtitlecomposer Core Widgets)

target_link_libraries(subtitlecomposer
	KF5::CoreAddons KF5::WidgetsAddons KF5::TextWidgets KF5::Codecs
	KF5::SonnetCore KF5::SonnetUi
	KF5::KrossCore KF5::KrossUi
	KF5::KDELibs4Support
	${PHONON_LIBRARY}
	${common_LIBS}
	${core_LIBS}
	${formats_LIBS}
	${services_LIBS}
	${widgets_LIBS}
)

add_definitions(
	${common_DEFS}
	${core_DEFS}
	${formats_DEFS}
	${services_DEFS}
	${widgets_DEFS}
)

install(TARGETS subtitlecomposer DESTINATION ${BIN_INSTALL_DIR})

install(FILES subtitlecomposerrc DESTINATION ${CONFIG_INSTALL_DIR})
install(FILES subtitlecomposerui.rc DESTINATION ${DATA_INSTALL_DIR}/subtitlecomposer)
install(FILES subtitlecomposer.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES subtitlecomposer.xml DESTINATION ${XDG_MIME_INSTALL_DIR})
