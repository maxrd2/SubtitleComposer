cmake_policy(SET CMP0048 NEW)
project(subtitlecomposer VERSION "0.7.1")

cmake_minimum_required(VERSION 3.4...3.20 FATAL_ERROR)
set(QT_MIN_VERSION "5.9.0")
set(KF5_MIN_VERSION "5.44.0")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)

include(ECMInstallIcons)
include(ECMSetupVersion)
include(ECMAddTests) # build tests

include(FeatureSummary)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
	Core Widgets Gui Test Qml)

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
	Auth Config ConfigWidgets CoreAddons I18n KIO XmlGui
	Sonnet Codecs TextWidgets WidgetsAddons)

#PO_SUBDIR
add_subdirectory(src)

add_custom_target(nsis COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/pkg/mingw/nsi-installer.sh" `${CMAKE_C_COMPILER} -dumpmachine`)
