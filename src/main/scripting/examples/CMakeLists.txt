FILE( GLOB ruby_examples *.rb )
FILE( GLOB python_examples *.py )
FILE( GLOB javascript_examples *.js )
FILE( GLOB header_files ../scripting_*.h )

INSTALL(
	FILES
		${ruby_examples}
		${python_examples}
		${javascript_examples}
		README
	DESTINATION
		${DATA_INSTALL_DIR}/subtitlecomposer/scripts
)

INSTALL(
	FILES
		${header_files}
	DESTINATION
		${DATA_INSTALL_DIR}/subtitlecomposer/scripts/api
)
