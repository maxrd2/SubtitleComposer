#ifndef CONFIG_H
#define CONFIG_H

#define SC_INSTALL_PREFIX "@CMAKE_INSTALL_PREFIX@"
#define SC_INSTALL_BIN "@CMAKE_INSTALL_PREFIX@/@BIN_INSTALL_DIR@"
#define SC_INSTALL_PLUGIN "@SC_PLUGIN_INSTALL_DIR@"

// Building AppImage distro
#cmakedefine SC_APPIMAGE

// Whether system icon theme is bundled
#cmakedefine SC_BUNDLE_SYSTEM_THEME

// Whether ICU was found
#cmakedefine HAVE_ICU

// fallback path to locate icons under non-KDE desktop environments
#define CUSTOM_ICON_INSTALL_PATH "@CMAKE_INSTALL_PREFIX@/@DATA_INSTALL_DIR@/subtitlecomposer/icons"

#endif
