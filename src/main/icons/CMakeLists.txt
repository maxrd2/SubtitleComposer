file(GLOB app_icons [0-9]*-subtitlecomposer.png [0-9]*-subtitlecomposer.svgz)

ecm_install_icons(ICONS ${app_icons}
	DESTINATION ${ICON_INSTALL_DIR}
	THEME hicolor)

file(GLOB app_icons [a-z]*.png [a-z]*.svgz [a-z]*.svg)
install(FILES ${app_icons}
	DESTINATION ${DATA_INSTALL_DIR}/subtitlecomposer/pics)
