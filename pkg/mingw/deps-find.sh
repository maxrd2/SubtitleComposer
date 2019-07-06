#!/bin/bash

set -e

arch="$1"
files=("${@:2}")

_usage="Usage: nsi-installer.sh <arch> <exe>\n"
[[ -z "$files" || -z "$arch" ]] && echo -e "$_usage" 1>&2 && exit 1
for f in "${files[@]}"; do [[ ! -f "$f" ]] && echo -e "ERROR: File '$f' doesn't exist!\n" 1>&2 && exit 1 ; done

processed=()
exclude=(
	advapi32.dll
	bcrypt.dll
	comdlg32.dll
	crypt32.dll
	dnsapi.dll
	dwmapi.dll
	gdi32.dll
	imm32.dll
	iphlpapi.dll
	kernel32.dll
	mpr.dll
	msimg32.dll
	msvcrt.dll
	netapi32.dll
	ole32.dll
	oleaut32.dll
	opengl32.dll
	secur32.dll
	setupapi.dll
	shell32.dll
	shlwapi.dll
	user32.dll
	userenv.dll
	uxtheme.dll
	usp10.dll
	version.dll
	winmm.dll
	winspool.drv
	ws2_32.dll
	wtsapi32.dll
)

debug() {
	echo -e "$@" | sed -Ee 's|(/.+)$|\x1b[1;39m\1\x1b[m|;s|(WARNING.+)$|\x1b[1;33m\1\x1b[m|;s|(ERROR.+)$|\x1b[1;31m\1\x1b[m|' 1>&2
}

finddeps() {
	deps=(`objdump -p "$1" | grep 'DLL Name' | sed -Ee 's|^.*DLL Name:\s([[:alnum:]].*[[:alnum:]])\s*$|\1|'`)
	for dll in "${deps[@]}"; do
		dll="${dll,,}"
		debug -n "$2'$dll'... "
		# skip dlls we have already processed
		[[ " ${processed[@]} " == *" $dll "* ]] && debug 'processed' && continue
		# skip dlls we don't want
		[[ " ${exclude[@]} " == *" $dll "* ]] && debug 'excluded' && continue
		
		# find full dll path
		_dll=$(find "$(dirname "$1")/" "/usr/$arch/" -iname "$dll" 2>/dev/null | sort -u)
		processed+=("$dll")
		[[ -z "$_dll" ]] && debug "WARNING: Dependency '$dll' was not found." && continue
		echo "$_dll"
		debug "$_dll"
		finddeps "$_dll" "\t$2"
	done
}

for f in "${files[@]}"; do finddeps "$f" ; done
