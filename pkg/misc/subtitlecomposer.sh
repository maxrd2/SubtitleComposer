#!/bin/sh

appdir="$(dirname "$(readlink -f "$0")")"

export PATH="${appdir}/usr/bin:$PATH"
export LD_LIBRARY_PATH="${appdir}/usr/lib:$LD_LIBRARY_PATH"
export XDG_DATA_DIRS="${appdir}/usr/share:$XDG_DATA_DIRS"

export QT_PLUGIN_PATH="${appdir}/usr/plugins"
# QT_PLUGIN_PATH - setting system's plugin path will cause crashes on many platforms
# echo export QT_PLUGIN_PATH="${appdir}/usr/plugins:$QT_PLUGIN_PATH:/usr/lib/qt/plugins:/usr/lib/$(uname -m)-linux-gnu/qt5/plugins"
# echo QT_PLUGIN_PATH=$QT_PLUGIN_PATH

# export QT_DEBUG_PLUGINS=1

# Arch Linux KDE; KUbuntu 18.04/20.04/20.10/21.04 LiveCD
# XDG_SESSION_DESKTOP="KDE"
# XDG_CURRENT_DESKTOP="KDE"

# Ubuntu 18.04/20.04/20.10/21.04 LiveCD - had to QT_QPA_PLATFORMTHEME=gtk3 for file dialogs and -style=breeze|oxygen
# XDG_SESSION_DESKTOP="ubuntu"
# XDG_CURRENT_DESKTOP="ubuntu:GNOME"

# Ubuntu Mate 21.04 LiveCD - had to QT_QPA_PLATFORMTHEME=gtk3 for file dialogs and -style=breeze|oxygen
# XDG_SESSION_DESKTOP="mate"
# XDG_CURRENT_DESKTOP="MATE"

# ICON THEME:
# https://openapplibrary.org/dev-tutorials/qt-icon-themes

app="${appdir}/usr/bin/subtitlecomposer"

case "$XDG_CURRENT_DESKTOP" in
"MATE")
	exec "${app}" -platformtheme gtk3 -style=adwaita "$@"
	;;
"ubuntu:GNOME")
	exec "${app}" -platformtheme gtk3 -style=adwaita "$@"
	;;
"KDE")
	exec "${app}" "$@"
	;;
*)
	exec "${app}" -platformtheme gtk2 -style=oxygen "$@"
	;;
esac

