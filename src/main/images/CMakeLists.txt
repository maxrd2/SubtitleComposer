file(GLOB png_images *.png)
file(GLOB sgvz_images *.svgz)

install(FILES
		${png_images}
		${sgvz_images}
	DESTINATION
		${DATA_INSTALL_DIR}/subtitlecomposer/pics
)

ecm_install_icons(${ICON_INSTALL_DIR})
