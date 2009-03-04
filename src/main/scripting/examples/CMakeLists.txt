FILE( GLOB ruby_examples. *.rb )
FILE( GLOB python_examples *.rb )
FILE( GLOB javascript_examples *.py )

INSTALL(
	FILES
		${ruby_examples}
		${python_examples}
		${javascript_examples}
	DESTINATION
		${DATA_INSTALL_DIR}/subtitlecomposer/scripts
)
