set(app_icons
	16-apps-subtitlecomposer.png
	32-apps-subtitlecomposer.png
	128-apps-subtitlecomposer.png
	256-apps-subtitlecomposer.png
)
ecm_install_icons(ICONS ${app_icons}
	DESTINATION ${ICON_INSTALL_DIR}
	THEME hicolor)

set(custom_icons
	128-categories-waveform.png
	16-actions-select_stream.png
	16-actions-time_from_video.png
	48-actions-audio_extract.png
	48-apps-gstreamer.png
	48-apps-mediaplayer.png
	48-apps-mplayer.png
	48-apps-xine.png
	64-apps-mpv.png
	sc-actions-anchor.svg
	sc-actions-current_line_follows_video.svg
	sc-actions-playrate_minus.svg
	sc-actions-playrate_plus.svg
	sc-actions-set_hide_time.svg
	sc-actions-set_show_time.svg
)
ecm_install_icons(ICONS ${custom_icons}
	DESTINATION ${DATA_INSTALL_DIR}/subtitlecomposer/icons
	THEME hicolor)
