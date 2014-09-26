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

INCLUDE_DIRECTORIES(
	${CMAKE_CURRENT_BINARY_DIR} # so files can find config.h
	${CMAKE_CURRENT_SOURCE_DIR} # so that we can refer to our files starting at src dir
	${KDE4_INCLUDES} # so we don't need to include it in other files
	${QT4_INCLUDES} # same as previous
)

ADD_DEFINITIONS(${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -fexceptions -DHAVE_CONFIG_H)
add_definitions(-D_DEFAULT_SOURCE)

# the subdirs must be added in a specific order
# if one dir uses code from another, its CMakeLists.txt will reference variables
# defined in the dependency directory that must be created before the former
# CMakeLists.txt is processed

ADD_SUBDIRECTORY(common)
ADD_SUBDIRECTORY(core)
ADD_SUBDIRECTORY(formats)
ADD_SUBDIRECTORY(config) # depends on common
ADD_SUBDIRECTORY(widgets)
ADD_SUBDIRECTORY(services)
ADD_SUBDIRECTORY(main)

# do the configuration of config.h at the end, so all the necessary variables have been set
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

