FILE(GLOB javascript_examples examples/*.js)
FILE(GLOB header_files scripting_*.h)

INSTALL(FILES
	${javascript_examples} examples/README
	DESTINATION ${DATA_INSTALL_DIR}/subtitlecomposer/scripts
)

INSTALL(FILES
	${header_files}
	DESTINATION ${DATA_INSTALL_DIR}/subtitlecomposer/scripts/api
)
