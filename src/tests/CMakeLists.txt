set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

include_directories(
	${QtTest_INCLUDE_DIRS}
)

add_executable(test-core-rangelist rangelisttest.cpp)
add_test(core-rangelist test-core-rangelist)
ecm_mark_as_test(test-core-rangelist)
target_link_libraries(test-core-rangelist Qt${QT_MAJOR_VERSION}::Test subtitlecomposer-lib)

add_executable(test-core-range rangetest.cpp)
add_test(core-range test-core-range)
ecm_mark_as_test(test-core-range)
target_link_libraries(test-core-range Qt${QT_MAJOR_VERSION}::Test subtitlecomposer-lib)

add_executable(test-core-time timetest.cpp)
add_test(core-time test-core-time)
ecm_mark_as_test(test-core-time)
target_link_libraries(test-core-time Qt${QT_MAJOR_VERSION}::Test subtitlecomposer-lib)

add_executable(test-core-richstring richstringtest.cpp)
add_test(core-richstring test-core-richstring)
ecm_mark_as_test(test-core-richstring)
target_link_libraries(test-core-richstring Qt${QT_MAJOR_VERSION}::Test subtitlecomposer-lib)

add_executable(test-core-richdocument richdocumenttest.cpp)
add_test(core-richdocument test-core-richdocument)
ecm_mark_as_test(test-core-richdocument)
target_link_libraries(test-core-richdocument Qt${QT_MAJOR_VERSION}::Test subtitlecomposer-lib)

add_executable(test-core-richdocumentlayout richdocumentlayouttest.cpp)
add_test(core-richdocument test-core-richdocumentlayout)
ecm_mark_as_test(test-core-richdocumentlayout)
target_link_libraries(test-core-richdocumentlayout Qt${QT_MAJOR_VERSION}::Test subtitlecomposer-lib)

add_executable(test-core-richcss richcsstest.cpp)
add_test(core-richcss test-core-richcss)
ecm_mark_as_test(test-core-richcss)
target_link_libraries(test-core-richcss Qt${QT_MAJOR_VERSION}::Test subtitlecomposer-lib)

add_executable(test-core-subtitle subtitletest.cpp)
add_test(core-subtitle test-core-subtitle)
ecm_mark_as_test(test-core-subtitle)
target_link_libraries(test-core-subtitle Qt${QT_MAJOR_VERSION}::Test subtitlecomposer-lib)
