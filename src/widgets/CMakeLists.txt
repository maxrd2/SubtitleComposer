set(widgets_INCLUDE_DIR
	${CMAKE_CURRENT_SOURCE_DIR}/
	CACHE PATH EXPORTEDVARIABLE)

set(widgets_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/attachablewidget.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/layeredwidget.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/pointingslider.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/simplerichtextedit.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/textoverlaywidget.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/timeedit.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/timevalidator.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/treeview.cpp
	CACHE INTERNAL EXPORTEDVARIABLE)
