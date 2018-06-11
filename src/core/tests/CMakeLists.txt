set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

include_directories(
	${helpers_INCLUDE_DIR}
	${core_INCLUDE_DIR}
	${Qt5Test_INCLUDE_DIRS}
)

set(rangelisttest_SRCS rangelisttest.cpp)
add_executable(core-rangelisttest ${rangelisttest_SRCS})
add_test(subtitlecomposer core-rangelisttest)
ecm_mark_as_test(core-rangelisttest)
target_link_libraries(core-rangelisttest ${subtitlecomposer_LIBS} Qt5::Core Qt5::Test)

set(rangetest_SRCS rangetest.cpp)
add_executable(core-rangetest ${rangetest_SRCS})
add_test(subtitlecomposer core-rangetest)
ecm_mark_as_test(core-rangetest)
target_link_libraries(core-rangetest ${subtitlecomposer_LIBS} Qt5::Core Qt5::Test)

set(timetest_SRCS ../time.cpp timetest.cpp)
add_executable(core-timetest ${timetest_SRCS})
add_test(subtitlecomposer core-timetest)
ecm_mark_as_test(core-timetest)
target_link_libraries(core-timetest ${subtitlecomposer_LIBS} Qt5::Core Qt5::Test)

set(sstringtest_SRCS ../sstring.cpp sstringtest.cpp)
add_executable(core-sstringtest ${sstringtest_SRCS})
add_test(subtitlecomposer core-sstringtest)
ecm_mark_as_test(core-sstringtest)
target_link_libraries(core-sstringtest ${subtitlecomposer_LIBS} Qt5::Core Qt5::Test)
