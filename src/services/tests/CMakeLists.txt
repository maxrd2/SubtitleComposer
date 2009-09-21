SET( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

SET( wavewritertest_SRCS wavewritertest.cpp )

KDE4_ADD_UNIT_TEST(
	services-wavewritertest
	${wavewritertest_SRCS}
	${common_SRCS}
	${config_SRCS}
	${services_SRCS}
)

INCLUDE_DIRECTORIES(
	${common_INCLUDE_DIR}
	${config_INCLUDE_DIR}
	${services_INCLUDE_DIR}
)

TARGET_LINK_LIBRARIES(
	services-wavewritertest
	${QT_QTTEST_LIBRARY}
	${KDE4_KDEUI_LIBS}
	${KDE4_KHTML_LIBS}
	${KDE4_KFILE_LIBS}
	${KDE4_PHONON_LIBS}
	${common_LIBS}
	${config_LIBS}
	${services_LIBS}
)
