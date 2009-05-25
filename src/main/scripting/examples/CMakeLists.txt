ADD_SUBDIRECTORY( api )

FILE( GLOB ruby_examples *.rb )
FILE( GLOB python_examples *.rb )
FILE( GLOB javascript_examples *.js )

INSTALL(
	FILES
		${ruby_examples}
		${python_examples}
		${javascript_examples}
		README
	DESTINATION
		${DATA_INSTALL_DIR}/subtitlecomposer/scripts
)
