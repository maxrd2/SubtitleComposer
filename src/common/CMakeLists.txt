set(common_LIBS
	KF5::CoreAddons KF5::WidgetsAddons KF5::TextWidgets KF5::Codecs
	KF5::SonnetCore KF5::SonnetUi
	KF5::KIOCore KF5::KIOFileWidgets KF5::KIOWidgets KF5::KIONTLM
	KF5::KrossCore KF5::KrossUi KF5::XmlGui KF5::I18n KF5::KHtml
	CACHE INTERNAL EXPORTEDVARIABLE
)

set(common_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/commondefs.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/qxtsignalwaiter.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/fileloadhelper.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/filesavehelper.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/filetrasher.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/languagecode.cpp
	CACHE INTERNAL EXPORTEDVARIABLE
)
