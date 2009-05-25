FILE( GLOB header_files *.h )

INSTALL(
	FILES
		${header_files}
	DESTINATION
		${DATA_INSTALL_DIR}/subtitlecomposer/scripts/api
)
