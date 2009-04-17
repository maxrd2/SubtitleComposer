FILE( GLOB png_images *.png )
FILE( GLOB sgvz_images *.svgz )

INSTALL(
	FILES
		${png_images}
		${sgvz_images}
	DESTINATION
		${DATA_INSTALL_DIR}/subtitlecomposer/pics
)

KDE4_INSTALL_ICONS( ${ICON_INSTALL_DIR} )
