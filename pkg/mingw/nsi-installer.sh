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
#	"/usr/$_arch/bin/dbus-daemon.exe" # causes the app to hang on exit?
	"/usr/$_arch/bin/kdeinit5.exe"
	"/usr/$_arch/bin/kwrapper5.exe"
	"/usr/$_arch/bin/klauncher.exe"
	"/usr/$_arch/bin/kiod5.exe"
	"/usr/$_arch/bin/kioslave5.exe"
)

dlls=(
	"${deps[@]}"
	`"$sdir/deps-find.sh" "$_arch" "$_destdir/bin/subtitlecomposer.exe" "${deps[@]}"`
)

for dll in "${dlls[@]}"; do
	dest="$(echo "$dll" | sed -Ee "s|^/usr/$_arch/lib/qt/plugins/|$_destdir/bin/|;s|^/usr/$_arch/|$_destdir/|")"
	install $_v -D -T "$dll" "$dest"
done

install $_v -d "$_destdir/share/dbus-1"
cp $_v -rf "/usr/$_arch/share/dbus-1" "$_destdir/share/"
rm $_v -rf "$_destdir/bin/data/icons" "$_destdir/bin/data/subtitlecomposer/icons"

localedest="$_destdir/bin/data/locale"
for f in $(pacman -Ql $(pacman -Qg kf5|cut -d ' ' -f 2-) | cut -d ' ' -f 2-|grep 'usr/share/locale.*.qm'); do
	install $_v "$f" -D "$localedest/${f/\/usr\/share\/locale\//}"
done

sed -e "s|{BUILD_PATH}|$_destdir|g" "$sdir/installer.nsi" > installer.nsi
makensis -V4 installer.nsi
