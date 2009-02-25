SET( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

SET( rangelisttest_SRCS rangelisttest.cpp )
KDE4_ADD_UNIT_TEST( core-rangelisttest ${rangelisttest_SRCS} )
TARGET_LINK_LIBRARIES( core-rangelisttest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} )

SET( rangetest_SRCS rangetest.cpp )
KDE4_ADD_UNIT_TEST( core-rangetest ${rangetest_SRCS} )
TARGET_LINK_LIBRARIES( core-rangetest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} )

SET( timetest_SRCS ../time.cpp timetest.cpp )
KDE4_ADD_UNIT_TEST( core-timetest ${timetest_SRCS} )
TARGET_LINK_LIBRARIES( core-timetest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} )

SET( sstringtest_SRCS ../sstring.cpp sstringtest.cpp )
KDE4_ADD_UNIT_TEST( core-sstringtest ${sstringtest_SRCS} )
TARGET_LINK_LIBRARIES( core-sstringtest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} )
