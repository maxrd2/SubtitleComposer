ADD_SUBDIRECTORY( configs )
ADD_SUBDIRECTORY( dialogs )
ADD_SUBDIRECTORY( images )
ADD_SUBDIRECTORY( actions )
ADD_SUBDIRECTORY( utils )
ADD_SUBDIRECTORY( scripting )

INCLUDE_DIRECTORIES(
	${common_INCLUDE_DIR}
	${config_INCLUDE_DIR}
	${core_INCLUDE_DIR}
	${formats_INCLUDE_DIR}
	${players_INCLUDE_DIR}
	${widgets_INCLUDE_DIR}
)

SET( subtitlecomposer_SRCS
	${common_SRCS}
	${config_SRCS}
	${core_SRCS}
	${formats_SRCS}
	${players_SRCS}
	${widgets_SRCS}
	${main_configs_SRCS}
	${main_dialogs_SRCS}
	${main_actions_SRCS}
	${main_utils_SRCS}
	${main_scripting_SRCS}
	${CMAKE_CURRENT_SOURCE_DIR}/application.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/audiolevelswidget.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/configdialog.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/currentlinewidget.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/errorsdialog.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/errorswidget.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/lineswidget.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/mainwindow.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/playerwidget.cpp
	CACHE INTERNAL EXPORTEDVARIABLE
)

KDE4_ADD_EXECUTABLE( subtitlecomposer ${subtitlecomposer_SRCS} )

TARGET_LINK_LIBRARIES(
	subtitlecomposer
	${KDE4_KDEUI_LIBS}
	${KDE4_KHTML_LIBS}
	${KDE4_KFILE_LIBS}
	${KDE4_KROSSUI_LIBS}
	${common_LIBS}
	${config_LIBS}
	${core_LIBS}
	${formats_LIBS}
	${players_LIBS}
	${widgets_LIBS}
)

ADD_DEFINITIONS(
	${common_DEFS}
	${config_DEFS}
	${core_DEFS}
	${formats_DEFS}
	${players_DEFS}
	${widgets_DEFS}
)

INSTALL( TARGETS subtitlecomposer DESTINATION ${BIN_INSTALL_DIR} )

INSTALL( FILES subtitlecomposerrc DESTINATION ${CONFIG_INSTALL_DIR} )
INSTALL( FILES subtitlecomposerui.rc DESTINATION ${DATA_INSTALL_DIR}/subtitlecomposer )
INSTALL( FILES subtitlecomposer.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
INSTALL( FILES subtitlecomposer.xml DESTINATION ${XDG_MIME_INSTALL_DIR} )
