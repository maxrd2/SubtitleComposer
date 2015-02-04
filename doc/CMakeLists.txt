include_directories(${QT_INCLUDES})

kde4_create_handbook(index.docbook INSTALL_DESTINATION ${HTML_INSTALL_DIR}/en)

add_subdirectory(en)
