if(NOT CMAKE_BUILD_TYPE)
	message(STATUS "No build type selected, default to Release")
	set(CMAKE_BUILD_TYPE "Release")
endif()

if(CMAKE_COMPILER_IS_GNUCC)
	message(STATUS "GNU C compiler detected")
	if(SC_NO_DEPRECATED)
		set(CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wno-deprecated-declarations")
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wno-deprecated-declarations")
	endif()
	if(SC_WARN_ERRORS)
		set(CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Werror")
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Werror")
	endif()
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Og -g")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Og -g")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -O3")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -O3")
	set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELEASE} -Wall -O3 -g")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELEASE} -Wall -O3 -g")
endif()

ecm_setup_version(${PROJECT_VERSION}
	VARIABLE_PREFIX SUBTITLECOMPOSER
	SOVERSION ${PROJECT_VERSION_MAJOR}
	VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/config-subtitlecomposer.h"
)

add_definitions(-D_DEFAULT_SOURCE)
if(APPIMAGE)
	message(STATUS "AppImage build enabled")
	set(SC_APPIMAGE 1 CACHE INTERNAL EXPORTEDVARIABLE)
endif()

find_package(FFmpeg 59.27.100 REQUIRED)
find_package(OpenAL REQUIRED)
find_package(ICU)

set(SC_PLUGIN_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/${KDE_INSTALL_LIBDIR}/subtitlecomposer" CACHE STRING EXPORTEDVARIABLE)
message(STATUS "Subtitle Composer plugin path: ${SC_PLUGIN_INSTALL_DIR}")

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR} # so that we can refer to our files starting at src dir
	${CMAKE_CURRENT_SOURCE_DIR}/widgets # needed for .ui files to access custom widgets
)

set(subtitlecomposer_SRCS
	#[[ main ]] application.cpp appglobal.cpp application_actions.cpp application_errorcheck.cpp application_subtitle.cpp mainwindow.cpp
	#[[ actions ]] actions/useraction.cpp actions/useractionnames.h actions/kcodecactionext.cpp actions/krecentfilesactionext.cpp
	#[[ configs ]] configs/configdialog.cpp configs/errorsconfigwidget.cpp configs/generalconfigwidget.cpp configs/playerconfigwidget.cpp configs/waveformconfigwidget.cpp
	#[[ core ]] core/formatdata.h core/range.h core/rangelist.h core/time.cpp core/richstring.cpp
	core/subtitle.cpp core/subtitleiterator.cpp core/subtitleline.cpp
	#[[ core/richtext ]] core/richtext/richdocument.cpp core/richtext/richdocumenteditor.cpp core/richtext/richdocumentlayout.cpp core/richtext/richcss.cpp
	core/richtext/richdom.cpp
	#[[ core/undo ]] core/undo/subtitleactions.cpp core/undo/subtitlelineactions.cpp core/undo/undoaction.cpp core/undo/undostack.cpp
	#[[ dialogs ]] dialogs/actiondialog.cpp #[[dialogs/actionwitherrortargetsdialog.cpp]] dialogs/actionwithtargetdialog.cpp
	dialogs/adjusttimesdialog.cpp dialogs/autodurationsdialog.cpp dialogs/changeframeratedialog.cpp dialogs/changetextscasedialog.cpp
	dialogs/durationlimitsdialog.cpp dialogs/encodingdetectdialog.cpp dialogs/fixoverlappingtimesdialog.cpp dialogs/fixpunctuationdialog.cpp
	dialogs/insertlinedialog.cpp dialogs/intinputdialog.cpp dialogs/joinsubtitlesdialog.cpp dialogs/progressdialog.cpp
	dialogs/removelinesdialog.cpp dialogs/selectablesubtitledialog.cpp dialogs/shifttimesdialog.cpp dialogs/smarttextsadjustdialog.cpp
	dialogs/splitsubtitledialog.cpp dialogs/subtitleclassdialog.cpp dialogs/subtitlecolordialog.cpp dialogs/subtitlevoicedialog.cpp
	dialogs/syncsubtitlesdialog.cpp dialogs/textinputdialog.cpp
	#[[ errors ]] errors/errorfinder.cpp errors/errortracker.cpp errors/finderrorsdialog.cpp
	#[[ formats ]] formats/format.h formats/formatmanager.h formats/inputformat.h formats/outputformat.h formats/formatmanager.cpp
	formats/microdvd/microdvdinputformat.h formats/microdvd/microdvdoutputformat.h
	formats/mplayer/mplayerinputformat.h formats/mplayer/mplayeroutputformat.h
	formats/mplayer2/mplayer2inputformat.h formats/mplayer2/mplayer2outputformat.h
	formats/subrip/subripinputformat.h formats/subrip/subripoutputformat.h
	formats/substationalpha/substationalphainputformat.h formats/substationalpha/substationalphaoutputformat.h
	formats/subviewer1/subviewer1inputformat.h formats/subviewer1/subviewer1outputformat.h
	formats/subviewer2/subviewer2inputformat.h formats/subviewer2/subviewer2outputformat.h
	formats/textdemux/textdemux.cpp
	formats/tmplayer/tmplayerinputformat.h formats/tmplayer/tmplayeroutputformat.h
	formats/vobsub/vobsubinputformat.h formats/vobsub/vobsubinputinitdialog.cpp formats/vobsub/vobsubinputprocessdialog.cpp
	formats/webvtt/webvttinputformat.cpp formats/webvtt/webvttoutputformat.cpp
	formats/youtubecaptions/youtubecaptionsinputformat.h formats/youtubecaptions/youtubecaptionsoutputformat.h
	#[[ gui ]] gui/currentlinewidget.cpp gui/playerwidget.cpp
	#[[ gui/waveform ]] gui/waveform/waveformwidget.cpp gui/waveform/wavebuffer.cpp gui/waveform/zoombuffer.cpp gui/waveform/waverenderer.cpp
	gui/waveform/wavesubtitle.cpp
	#[[ gui/treeview ]] gui/treeview/linesitemdelegate.cpp gui/treeview/linesmodel.cpp gui/treeview/linesselectionmodel.cpp gui/treeview/lineswidget.cpp
	gui/treeview/richlineedit.cpp gui/treeview/richdocumentptr.cpp gui/treeview/treeview.cpp
	#[[ gui/subtitlemetawidget ]] gui/subtitlemeta/subtitlemetawidget.cpp gui/subtitlemeta/csshighlighter.cpp
	gui/subtitlemeta/subtitlepositionwidget.cpp
	#[[ helpers ]] helpers/commondefs.cpp helpers/debug.cpp helpers/languagecode.cpp
	helpers/pluginhelper.h
	#[[ scripting ]] scripting/scriptsmanager.cpp
	scripting/scripting_rangesmodule.cpp scripting/scripting_stringsmodule.cpp scripting/scripting_subtitlemodule.cpp scripting/scripting_subtitlelinemodule.cpp
	scripting/scripting_list.cpp scripting/scripting_range.cpp scripting/scripting_rangelist.cpp scripting/scripting_richstring.cpp scripting/scripting_subtitle.cpp
	scripting/scripting_subtitleline.cpp
	#[[ speechprocessor ]] speechprocessor/speechprocessor.cpp speechprocessor/speechplugin.cpp
	#[[ streamprocessor ]] streamprocessor/streamprocessor.cpp
	#[[ translations ]] translate/translatedialog.cpp translate/translateengine.cpp
	#[[ translation engines ]] translate/deeplengine.cpp translate/mintengine.cpp translate/googlecloudengine.cpp
	#[[ utils ]] utils/finder.cpp utils/replacer.cpp utils/speller.cpp
	#[[ videoplayer ]] videoplayer/videoplayer.cpp videoplayer/videowidget.cpp videoplayer/waveformat.h videoplayer/subtitletextoverlay.cpp
	videoplayer/backend/glrenderer.cpp videoplayer/backend/ffplayer.cpp videoplayer/backend/framequeue.cpp videoplayer/backend/packetqueue.cpp
	videoplayer/backend/decoder.cpp videoplayer/backend/audiodecoder.cpp videoplayer/backend/videodecoder.cpp videoplayer/backend/subtitledecoder.cpp
	videoplayer/backend/clock.cpp videoplayer/backend/streamdemuxer.cpp videoplayer/backend/renderthread.cpp videoplayer/backend/videostate.cpp
	#[[ widgets ]] widgets/attachablewidget.cpp widgets/layeredwidget.cpp widgets/pointingslider.cpp widgets/simplerichtextedit.cpp
	widgets/textoverlaywidget.cpp widgets/timeedit.cpp
	CACHE INTERNAL EXPORTEDVARIABLE
	)

kconfig_add_kcfg_files(subtitlecomposer_SRCS GENERATE_MOC scconfig.kcfgc)

ki18n_wrap_ui(subtitlecomposer_SRCS
	#[[ configs ]] configs/errorsconfigwidget.ui configs/generalconfigwidget.ui configs/playerconfigwidget.ui configs/waveformconfigwidget.ui
	#[[ dialogs ]] dialogs/encodingdetectdialog.ui dialogs/intinputdialog.ui dialogs/subtitleclassdialog.ui dialogs/subtitlevoicedialog.ui
	dialogs/textinputdialog.ui
	#[[ formats ]] formats/vobsub/vobsubinputinitdialog.ui formats/vobsub/vobsubinputprocessdialog.ui
	#[[ gui/subtitlemetawidget ]] gui/subtitlemeta/subtitlepositionwidget.ui
	#[[ scripting ]] scripting/scriptsmanager.ui
	#[[ translation engines ]] translate/deeplengine.ui translate/mintengine.ui translate/googlecloudengine.ui
	)

add_subdirectory(icons)

list(APPEND subtitlecomposer_RES ${CMAKE_CURRENT_SOURCE_DIR}/subtitlecomposer.qrc)

if(QT_MAJOR_VERSION EQUAL 6)
	qt6_add_resources(subtitlecomposer_RES_SRC ${subtitlecomposer_RES})
else()
	qt5_add_resources(subtitlecomposer_RES_SRC ${subtitlecomposer_RES})
endif()

add_library(subtitlecomposer-lib STATIC ${subtitlecomposer_SRCS})
target_link_libraries(subtitlecomposer-lib
	Qt${QT_MAJOR_VERSION}::Core Qt${QT_MAJOR_VERSION}::Widgets Qt${QT_MAJOR_VERSION}::WidgetsPrivate
	Qt${QT_MAJOR_VERSION}::Gui Qt${QT_MAJOR_VERSION}::Qml 
	KF${KF_MAJOR_VERSION}::CoreAddons KF${KF_MAJOR_VERSION}::WidgetsAddons KF${KF_MAJOR_VERSION}::TextWidgets KF${KF_MAJOR_VERSION}::Codecs
	KF${KF_MAJOR_VERSION}::SonnetCore KF${KF_MAJOR_VERSION}::SonnetUi
	KF${KF_MAJOR_VERSION}::KIOCore KF${KF_MAJOR_VERSION}::KIOFileWidgets KF${KF_MAJOR_VERSION}::KIOWidgets
	KF${KF_MAJOR_VERSION}::XmlGui KF${KF_MAJOR_VERSION}::I18n
	OpenSSL::SSL
	${FFMPEG_LIBRARIES}
	${OPENAL_LIBRARY}
	${ICU_LIBRARIES}
)
if(QT_MAJOR_VERSION EQUAL 6)
	target_link_libraries(subtitlecomposer-lib Qt::OpenGLWidgets Qt::Core5Compat)
endif()
target_include_directories(subtitlecomposer-lib PUBLIC
	${Qt${QT_MAJOR_VERSION}Core_INCLUDE_DIRS}
	${Qt${QT_MAJOR_VERSION}Widgets_INCLUDE_DIRS} ${Qt${QT_MAJOR_VERSION}Widgets_PRIVATE_INCLUDE_DIRS}
	${Qt${QT_MAJOR_VERSION}Gui_INCLUDE_DIRS} ${Qt${QT_MAJOR_VERSION}Gui_PRIVATE_INCLUDE_DIRS}
	${OPENAL_INCLUDE_DIR}
	${FFMPEG_INCLUDE_DIRS}
)
if(ICU_FOUND)
	message(STATUS "ICU Library will be used for charset detection.")
	set(HAVE_ICU 1 CACHE INTERNAL EXPORTEDVARIABLE)
	target_include_directories(subtitlecomposer-lib PUBLIC ${ICU_INCLUDE_DIRS})
else()
	message(STATUS "ICU Library not found. KEncodingProber fallback will be used for charset detection.")
endif()

add_subdirectory(tests)
add_subdirectory(scripting)

add_executable(subtitlecomposer WIN32 main.cpp ${subtitlecomposer_RES_SRC})
target_link_libraries(subtitlecomposer subtitlecomposer-lib)

install(TARGETS subtitlecomposer DESTINATION ${KDE_INSTALL_BINDIR})

install(FILES subtitlecomposerrc DESTINATION ${KDE_INSTALL_CONFDIR})
install(FILES org.kde.subtitlecomposer.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES org.kde.subtitlecomposer.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES subtitlecomposer.xml DESTINATION ${KDE_INSTALL_MIMEDIR})

# build plugins
add_subdirectory(speechplugins/pocketsphinx)

# do the configuration of config.h at the end, so all the necessary variables have been set
configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)
