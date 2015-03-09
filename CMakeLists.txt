project(subtitlecomposer)

cmake_minimum_required(VERSION 2.8.12)
set(QT_MIN_VERSION "5.2.0")

find_package(ECM 1.2.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)

include(ECMInstallIcons)

include(ECMAddTests) # build tests

include(FeatureSummary)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED Core Widgets Test Declarative)
#add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0)

find_package(KF5 REQUIRED
	Auth Config ConfigWidgets CoreAddons I18n KIO
	Sonnet Kross Codecs TextWidgets WidgetsAddons
	# TODO: remove it when done porting
	KDELibs4Support)

add_definitions(
#  -DQT_USE_QSTRINGBUILDER
#  -DQT_NO_CAST_TO_ASCII
#  -DQT_NO_CAST_FROM_ASCII
#  -DQT_STRICT_ITERATORS
  -DQT_NO_URL_CAST_FROM_STRING
#  -DQT_NO_CAST_FROM_BYTEARRAY
#  -DQT_NO_SIGNALS_SLOTS_KEYWORDS
#  -DQT_USE_FAST_OPERATOR_PLUS
#  -DQT_USE_FAST_CONCATENATION
#  -Wno-deprecated-declarations
)

add_subdirectory(po)
add_subdirectory(src)
