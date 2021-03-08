if(NOT CMAKE_BUILD_TYPE)
	message(STATUS "No build type selected, default to Release")
	set(CMAKE_BUILD_TYPE "Release")
endif()

if(CMAKE_COMPILER_IS_GNUCC)
	message(STATUS "GNU C compiler detected")
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

add_definitions(-DHAVE_CONFIG_H)
add_definitions(-D_DEFAULT_SOURCE)

find_package(FFmpeg 57.83.100 REQUIRED)
find_package(OpenAL REQUIRED)
find_package(ICU)

set(SC_PLUGIN_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/${KDE_INSTALL_LIBDIR}/subtitlecomposer" CACHE STRING EXPORTEDVARIABLE)
message(STATUS "Subtitle Composer plugin path: ${SC_PLUGIN_INSTALL_DIR}")

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR} # so that we can refer to our files starting at src dir
	${CMAKE_CURRENT_SOURCE_DIR}/widgets # needed for .ui files to access custom widgets
	${Qt5Core_INCLUDE_DIRS}
	${Qt5Widgets_INCLUDE_DIRS} ${Qt5Widgets_PRIVATE_INCLUDE_DIRS}
	${Qt5Gui_INCLUDE_DIRS} ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
	${OPENAL_INCLUDE_DIR}
	${FFMPEG_INCLUDE_DIRS}
)

add_subdirectory(tests)

add_subdirectory(scripting)

set(subtitlecomposer_SRCS
	#[[ main ]] application.cpp application_actions.cpp application_errorcheck.cpp application_subtitle.cpp main.cpp mainwindow.cpp
	#[[ actions ]] actions/useraction.cpp actions/useractionnames.h actions/kcodecactionext.cpp actions/krecentfilesactionext.cpp
	#[[ configs ]] configs/configdialog.cpp configs/errorsconfigwidget.cpp configs/generalconfigwidget.cpp configs/playerconfigwidget.cpp configs/waveformconfigwidget.cpp
	#[[ core ]] core/formatdata.h core/range.h core/rangelist.h core/time.cpp core/richdocument.cpp core/richdocumenteditor.cpp core/sstring.cpp
	core/subtitle.cpp core/subtitleiterator.cpp core/subtitleline.cpp core/undo/subtitleactions.cpp core/undo/subtitlelineactions.cpp
	core/undo/undoaction.cpp core/undo/undostack.cpp
	#[[ dialogs ]] dialogs/actiondialog.cpp #[[dialogs/actionwitherrortargetsdialog.cpp]] dialogs/actionwithtargetdialog.cpp dialogs/adjusttimesdialog.cpp
	dialogs/autodurationsdialog.cpp dialogs/changeframeratedialog.cpp dialogs/changetextscasedialog.cpp dialogs/durationlimitsdialog.cpp
	dialogs/encodingdetectdialog.cpp dialogs/fixoverlappingtimesdialog.cpp dialogs/fixpunctuationdialog.cpp dialogs/insertlinedialog.cpp
	dialogs/intinputdialog.cpp dialogs/joinsubtitlesdialog.cpp dialogs/progressdialog.cpp dialogs/removelinesdialog.cpp dialogs/selectablesubtitledialog.cpp
	dialogs/shifttimesdialog.cpp dialogs/smarttextsadjustdialog.cpp dialogs/splitsubtitledialog.cpp dialogs/subtitlecolordialog.cpp
	dialogs/syncsubtitlesdialog.cpp dialogs/textinputdialog.cpp dialogs/translatedialog.cpp
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
	formats/youtubecaptions/youtubecaptionsinputformat.h formats/youtubecaptions/youtubecaptionsoutputformat.h
	#[[ gui ]] gui/currentlinewidget.cpp gui/playerwidget.cpp gui/waveformwidget.cpp
	gui/treeview/linesitemdelegate.cpp gui/treeview/linesmodel.cpp gui/treeview/linesselectionmodel.cpp gui/treeview/lineswidget.cpp
	gui/treeview/richlineedit.cpp gui/treeview/richdocumentptr.cpp gui/treeview/treeview.cpp
	#[[ helpers ]] helpers/commondefs.cpp helpers/fileloadhelper.cpp helpers/filesavehelper.cpp helpers/filetrasher.cpp helpers/languagecode.cpp helpers/pluginhelper.h
	#[[ scripting ]] scripting/scriptsmanager.cpp
	scripting/scripting_rangesmodule.cpp scripting/scripting_stringsmodule.cpp scripting/scripting_subtitlemodule.cpp scripting/scripting_subtitlelinemodule.cpp
	scripting/scripting_list.cpp scripting/scripting_range.cpp scripting/scripting_rangelist.cpp scripting/scripting_sstring.cpp scripting/scripting_subtitle.cpp
	scripting/scripting_subtitleline.cpp
	#[[ speechprocessor ]] speechprocessor/speechprocessor.cpp speechprocessor/speechplugin.cpp
	#[[ streamprocessor ]] streamprocessor/streamprocessor.cpp
	#[[ utils ]] utils/finder.cpp utils/replacer.cpp utils/speller.cpp utils/translator.cpp utils/language.cpp
	#[[ videoplayer ]]
	videoplayer/videoplayer.cpp videoplayer/videowidget.cpp videoplayer/waveformat.h videoplayer/subtitletextoverlay.cpp
	videoplayer/backend/glrenderer.cpp videoplayer/backend/ffplayer.cpp videoplayer/backend/framequeue.cpp videoplayer/backend/packetqueue.cpp
	videoplayer/backend/decoder.cpp videoplayer/backend/audiodecoder.cpp videoplayer/backend/videodecoder.cpp videoplayer/backend/subtitledecoder.cpp
	videoplayer/backend/clock.cpp videoplayer/backend/streamdemuxer.cpp videoplayer/backend/renderthread.cpp videoplayer/backend/videostate.cpp
	#[[ widgets ]] widgets/attachablewidget.cpp widgets/layeredwidget.cpp widgets/pointingslider.cpp widgets/simplerichtextedit.cpp
	widgets/textoverlaywidget.cpp widgets/timeedit.cpp widgets/timevalidator.cpp
	CACHE INTERNAL EXPORTEDVARIABLE
	)

if(MINGW)
	qt5_add_resources(subtitlecomposer_SRCS subtitlecomposer.qrc)
	list(APPEND subtitlecomposer_SRCS subtitlecomposer.rc)
else()
	ecm_install_icons(ICONS
		icons/16-apps-subtitlecomposer.png icons/32-apps-subtitlecomposer.png icons/128-apps-subtitlecomposer.png icons/256-apps-subtitlecomposer.png
		DESTINATION ${ICON_INSTALL_DIR}
		THEME hicolor)

	ecm_install_icons(ICONS
		icons/128-categories-waveform.png icons/16-actions-select_stream.png icons/16-actions-time_from_video.png icons/48-actions-audio_extract.png
		icons/48-apps-mediaplayer.png icons/64-apps-pocketsphinx.png icons/sc-actions-anchor.svg icons/sc-actions-current_line_follows_video.svg
		icons/sc-actions-playrate_minus.svg icons/sc-actions-playrate_plus.svg icons/sc-actions-set_hide_time.svg icons/sc-actions-set_show_time.svg
		DESTINATION ${DATA_INSTALL_DIR}/subtitlecomposer/icons
		THEME hicolor)
endif()

kconfig_add_kcfg_files(subtitlecomposer_SRCS GENERATE_MOC scconfig.kcfgc)

ki18n_wrap_ui(subtitlecomposer_SRCS
	#[[ configs ]] configs/errorsconfigwidget.ui configs/generalconfigwidget.ui configs/playerconfigwidget.ui configs/waveformconfigwidget.ui
	#[[ dialogs ]] dialogs/encodingdetectdialog.ui dialogs/intinputdialog.ui dialogs/textinputdialog.ui
	#[[ formats ]] formats/vobsub/vobsubinputinitdialog.ui formats/vobsub/vobsubinputprocessdialog.ui
	#[[ scripting ]] scripting/scriptsmanager.ui
	)

add_executable(subtitlecomposer WIN32 ${subtitlecomposer_SRCS})

if(ICU_FOUND)
	message(STATUS "ICU Library will be used for charset detection.")
	set(HAVE_ICU 1 CACHE INTERNAL EXPORTEDVARIABLE)
	include_directories(${ICU_INCLUDE_DIRS})
else()
	message(STATUS "ICU Library not found. KEncodingProber fallback will be used for charset detection.")
endif()

target_link_libraries(subtitlecomposer
	Qt5::Core Qt5::Widgets Qt5::WidgetsPrivate Qt5::Gui
	KF5::CoreAddons KF5::WidgetsAddons KF5::TextWidgets KF5::Codecs
	KF5::SonnetCore KF5::SonnetUi
	KF5::KIOCore KF5::KIOFileWidgets KF5::KIOWidgets KF5::KIONTLM
	KF5::KrossCore KF5::KrossUi KF5::XmlGui KF5::I18n
	${FFMPEG_LIBRARIES}
	${OPENAL_LIBRARY}
	${ICU_LIBRARIES}
)

install(TARGETS subtitlecomposer DESTINATION ${BIN_INSTALL_DIR})

install(FILES subtitlecomposerrc DESTINATION ${CONFIG_INSTALL_DIR})
install(FILES subtitlecomposerui.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/subtitlecomposer)
install(FILES org.kde.subtitlecomposer.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES org.kde.subtitlecomposer.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES subtitlecomposer.xml DESTINATION ${XDG_MIME_INSTALL_DIR})

# build plugins
add_subdirectory(speechplugins/pocketsphinx)

# do the configuration of config.h at the end, so all the necessary variables have been set
configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

