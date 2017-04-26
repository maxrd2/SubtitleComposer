project(subtitlecomposer)

cmake_minimum_required(VERSION 2.8.12)
set(QT_MIN_VERSION "5.2.0")

#set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ECM 1.2.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

cmake_policy(SET CMP0043 NEW)
if(CMAKE_VERSION VERSION_GREATER 3.3.0)
	cmake_policy(SET CMP0063 NEW)
endif(CMAKE_VERSION VERSION_GREATER 3.3.0)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)

include(ECMInstallIcons)

include(ECMAddTests) # build tests

include(FeatureSummary)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED Core Widgets Test)

find_package(KF5 REQUIRED
	Auth Config ConfigWidgets CoreAddons I18n KIO KHtml XmlGui
	Sonnet Kross Codecs TextWidgets WidgetsAddons)

add_subdirectory(po)
add_subdirectory(src)
