project(subtitlecomposer)

cmake_minimum_required(VERSION 3.3 FATAL_ERROR)
set(QT_MIN_VERSION "5.3.0")
set(KF5_MIN_VERSION "5.2.0")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ECM 1.2.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

cmake_policy(SET CMP0043 NEW)
cmake_policy(SET CMP0063 NEW)
if(POLICY CMP0071)
	cmake_policy(SET CMP0071 NEW)
endif()

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)

include(ECMInstallIcons)

include(ECMAddTests) # build tests

include(FeatureSummary)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
	Core Widgets Gui Test
)

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
	Auth Config ConfigWidgets CoreAddons I18n KIO XmlGui
	Sonnet Kross Codecs TextWidgets WidgetsAddons)

add_subdirectory(po)
add_subdirectory(src)
