#ifndef CONFIG_H
#define CONFIG_H

#define SCPLUGIN_PATH "@SC_PLUGIN_INSTALL_DIR@"

// Whether MPV was found
#cmakedefine HAVE_MPV

// Whether Xine was found
#cmakedefine HAVE_XINE

// Whether XCB was found
#cmakedefine HAVE_XCB

// Whether ICU was found
#cmakedefine HAVE_ICU

// fallback path to locate icons under non-KDE desktop environments
#define CUSTOM_ICON_INSTALL_PATH "@CMAKE_INSTALL_PREFIX@/@DATA_INSTALL_DIR@/subtitlecomposer/icons"

#endif
