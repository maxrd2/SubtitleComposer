INCLUDE_DIRECTORIES(
	${CMAKE_CURRENT_BINARY_DIR} # so files can find config.h
	${CMAKE_CURRENT_SOURCE_DIR} # so that we can refer to our files starting at src dir
	${KDE4_INCLUDES} # so we don't need to include it in other files
	${QT_INCLUDES} # same as previous
)

ADD_DEFINITIONS( ${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -fexceptions -DHAVE_CONFIG_H )

# the subdirs must be added in a specific order
# if one dir uses code from another, its CMakeLists.txt will reference variables
# defined in the dependency directory that must be created before the former
# CMakeLists.txt is processed

ADD_SUBDIRECTORY( common )
ADD_SUBDIRECTORY( core )
ADD_SUBDIRECTORY( formats )
ADD_SUBDIRECTORY( config ) # depends on common
ADD_SUBDIRECTORY( widgets )
ADD_SUBDIRECTORY( player )
ADD_SUBDIRECTORY( main )

# do the configuration of config.h at the end, so all the necessary variables have been set
CONFIGURE_FILE( ${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )

