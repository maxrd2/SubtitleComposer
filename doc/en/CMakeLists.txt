include_directories( ${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )

kde4_create_handbook( index.docbook INSTALL_DESTINATION ${HTML_INSTALL_DIR}/en )
