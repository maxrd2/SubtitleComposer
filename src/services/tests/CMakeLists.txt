set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

set(wavewritertest_SRCS wavewritertest.cpp)

include_directories(
	${common_INCLUDE_DIR}
	${config_INCLUDE_DIR}
	${services_INCLUDE_DIR}
)

add_executable(services-wavewritertest
	${wavewritertest_SRCS}
	${common_SRCS}
	${config_SRCS}
	${services_SRCS})
add_test(subtitlecomposer services-wavewritertest)
ecm_mark_as_test(services-wavewritertest)
target_link_libraries(services-wavewritertest
	${common_LIBS}
	KF5::KDELibs4Support
	${PHONON_LIBRARY}
	${services_LIBS}
)
qt5_use_modules(services-wavewritertest Core Test)
