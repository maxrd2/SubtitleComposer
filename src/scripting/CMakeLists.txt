FILE(GLOB javascript_examples examples/*.js)
FILE(GLOB header_files scripting_*.h)

INSTALL(FILES
	${javascript_examples} examples/README
	DESTINATION ${KDE_INSTALL_DATADIR}/subtitlecomposer/scripts
)

INSTALL(FILES
	${header_files}
	DESTINATION ${KDE_INSTALL_DATADIR}/subtitlecomposer/scripts/api
)
