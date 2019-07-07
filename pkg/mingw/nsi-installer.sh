#!/bin/bash

set -e

_arch="$1"
_destdir="$DESTDIR/usr/$_arch"
sdir="$(dirname "$0")"
[[ -z "$DESTDIR" ]] && echo -e "ERROR: DESTDIR was not specified... bailing\n" && exit 1
[[ -z "$_arch" ]] && echo -e "Usage: nsi-installer.sh <arch>\n" && exit 1

_v= ; [[ $VERBOSE -eq 1 ]] && _v='-v'

rm -rf "$DESTDIR"
mkdir -p "$DESTDIR"
make install

echo "Searching $_arch dependencies..." 1>&2

deps=(
	"/usr/$_arch/lib/qt/plugins/platforms/qwindows.dll"
	"/usr/$_arch/lib/qt/plugins/audio/qtaudio_windows.dll"
	"/usr/$_arch/lib/qt/plugins/printsupport/windowsprintersupport.dll"
	"/usr/$_arch/lib/qt/plugins/styles/qwindowsvistastyle.dll"
	"/usr/$_arch/lib/qt/plugins/iconengines/"*.dll
	"/usr/$_arch/lib/qt/plugins/imageformats/qico.dll"
	"/usr/$_arch/lib/qt/plugins/imageformats/qjpeg.dll"
	"/usr/$_arch/lib/qt/plugins/imageformats/qsvg.dll"
	"/usr/$_arch/lib/qt/plugins/kcm_kio.dll"
	"/usr/$_arch/lib/qt/plugins/kf5/kio/"*.dll
	"/usr/$_arch/lib/qt/plugins/kf5/kiod/"*.dll
	"/usr/$_arch/lib/qt/plugins/kf5/sonnet/"*.dll
	"/usr/$_arch/bin/dbus-daemon.exe"
	"/usr/$_arch/bin/kdeinit5.exe"
	"/usr/$_arch/bin/kwrapper5.exe"
	"/usr/$_arch/bin/klauncher.exe"
	"/usr/$_arch/bin/kiod5.exe"
	"/usr/$_arch/bin/kioslave.exe"
	# GStreamer is not playing videos
	"/usr/$_arch/bin/libgst"*.dll
	"/usr/$_arch/lib/gstreamer-1.0/"*.dll
	"/usr/$_arch/lib/gstreamer-1.0/"*.exe # remove this
	"/usr/$_arch/bin/"*gst*.exe # remove this
	"/usr/$_arch/bin/mpv.exe" "/usr/$_arch/bin/mpv.com" # remove this
)

dlls=(
	"${deps[@]}"
	`"$sdir/deps-find.sh" "$_arch" "$_destdir/bin/subtitlecomposer.exe" "$_destdir/lib/subtitlecomposer"/*.dll "${deps[@]}"`
)

for dll in "${dlls[@]}"; do
	dest="$(echo "$dll" | sed -Ee "s|^/usr/$_arch/lib/qt/plugins/|$_destdir/bin/|;s|^/usr/$_arch/|$_destdir/|")"
	install $_v -D -T "$dll" "$dest"
done

install $_v -d "$_destdir/share/dbus-1"
cp $_v -rf "/usr/$_arch/share/dbus-1" "$_destdir/share/"
#install $_v -d "$_destdir/bin/data/icons/breeze"
#cp $_v -rf "/usr/share/icons/breeze/index.theme" "$_destdir/bin/data/icons/breeze"
#cp $_v -rf "/usr/share/icons/breeze/actions" "$_destdir/bin/data/icons/breeze"
rm $_v -rf "$_destdir/bin/data/icons" "$_destdir/bin/data/subtitlecomposer/icons"

sed -e "s|{BUILD_PATH}|$_destdir|g" "$sdir/installer.nsi" > installer.nsi
makensis -V4 installer.nsi
