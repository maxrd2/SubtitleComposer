set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

include_directories(
	${Qt5Test_INCLUDE_DIRS}
)

add_executable(test-core-rangelist rangelisttest.cpp)
add_test(subtitlecomposer test-core-rangelist)
ecm_mark_as_test(test-core-rangelist)
target_link_libraries(test-core-rangelist Qt5::Core Qt5::Test)

add_executable(test-core-range rangetest.cpp)
add_test(subtitlecomposer test-core-range)
ecm_mark_as_test(test-core-range)
target_link_libraries(test-core-range Qt5::Core Qt5::Test)

add_executable(test-core-time timetest.cpp ../core/time.cpp)
add_test(subtitlecomposer test-core-time)
ecm_mark_as_test(test-core-time)
target_link_libraries(test-core-time Qt5::Core Qt5::Test)

add_executable(test-core-sstring sstringtest.cpp ../core/sstring.cpp)
add_test(subtitlecomposer test-core-sstring)
ecm_mark_as_test(test-core-sstring)
target_link_libraries(test-core-sstring Qt5::Core Qt5::Widgets Qt5::Test)

add_executable(test-core-richdocument richdocumenttest.cpp ../core/richdocument.cpp ../core/sstring.cpp)
add_test(subtitlecomposer test-core-richdocument)
ecm_mark_as_test(test-core-richdocument)
target_link_libraries(test-core-richdocument Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Test)
