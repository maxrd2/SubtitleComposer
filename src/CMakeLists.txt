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

find_package(Phonon4Qt5 4.6.60 QUIET REQUIRED NO_MODULE)
set_package_properties(Phonon4Qt5 PROPERTIES
   DESCRIPTION "Qt-based audio library"
   TYPE REQUIRED)

find_package(X11 REQUIRED)

# the subdirs must be added in a specific order
# if one dir uses code from another, its CMakeLists.txt will reference variables
# defined in the dependency directory that must be created before the former
# CMakeLists.txt is processed

set(SC_PLUGIN_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/${LIB_INSTALL_DIR}/subtitlecomposer CACHE INTERNAL EXPORTEDVARIABLE)

add_subdirectory(common)
add_subdirectory(core)
add_subdirectory(formats)
add_subdirectory(widgets)
add_subdirectory(videoplayer)
add_subdirectory(streamprocessor)
add_subdirectory(speechprocessor)
add_subdirectory(main)

add_subdirectory(videoplayerplugins)

add_subdirectory(speechplugins)

# do the configuration of config.h at the end, so all the necessary variables have been set
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

