set(main_configs_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/configdialog.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/errorsconfigwidget.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/generalconfigwidget.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/playerconfigwidget.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/waveformconfigwidget.cpp
	CACHE INTERNAL EXPORTEDVARIABLE
)

set(main_configs_UIS
	${CMAKE_CURRENT_SOURCE_DIR}/errorsconfigwidget.ui
	${CMAKE_CURRENT_SOURCE_DIR}/generalconfigwidget.ui
	${CMAKE_CURRENT_SOURCE_DIR}/playerconfigwidget.ui
	${CMAKE_CURRENT_SOURCE_DIR}/waveformconfigwidget.ui
	CACHE INTERNAL EXPORTEDVARIABLE
)
