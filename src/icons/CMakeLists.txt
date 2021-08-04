list(APPEND subtitlecomposer_RES ${CMAKE_CURRENT_SOURCE_DIR}/icon-bundle.qrc)

if(MINGW)
	list(APPEND subtitlecomposer_RES_SRC ${CMAKE_CURRENT_SOURCE_DIR}/subtitlecomposer.rc)
	set(BUNDLE_THEME_ICONS 1)
elseif(APPIMAGE)
	ecm_install_icons(ICONS
		256-apps-subtitlecomposer.png
		DESTINATION ${ICON_INSTALL_DIR}
		THEME hicolor)
	set(BUNDLE_THEME_ICONS 1)
else()
	ecm_install_icons(ICONS
		16-apps-subtitlecomposer.png 32-apps-subtitlecomposer.png 128-apps-subtitlecomposer.png 256-apps-subtitlecomposer.png
		DESTINATION ${ICON_INSTALL_DIR}
		THEME hicolor)
endif()

if(BUNDLE_THEME_ICONS)
	if(BUNDLE_THEME_ICONS EQUAL 1)
		if(CMAKE_CROSSCOMPILING)
			set(BUNDLE_THEME_ICONS "/usr/share/icons/breeze") # how do you get host's install_prefix when cross compiling?
		else()
			set(BUNDLE_THEME_ICONS "${CMAKE_INSTALL_PREFIX}/${ICON_INSTALL_DIR}/breeze")
		endif()
	endif()
	message(STATUS "Bundling system icon theme...'")
	execute_process(COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/theme-icons-bundle.sh" "${BUNDLE_THEME_ICONS}"
		OUTPUT_FILE ${CMAKE_CURRENT_BINARY_DIR}/theme-icons-bundle.qrc)
	list(APPEND subtitlecomposer_RES ${CMAKE_CURRENT_BINARY_DIR}/theme-icons-bundle.qrc)
	set(SC_BUNDLE_SYSTEM_THEME 1 CACHE INTERNAL EXPORTEDVARIABLE)
endif()

# export vars
set(subtitlecomposer_RES_SRC "${subtitlecomposer_RES_SRC}" PARENT_SCOPE)
set(subtitlecomposer_RES "${subtitlecomposer_RES}" PARENT_SCOPE)
