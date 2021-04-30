#!/bin/bash

set -e

_rt="$(cd "$(dirname "$0")/../.." && pwd)"

icons=(`grep -hr QIcon::fromTheme "$_rt/src" \
	| sed -n -E -e '/.*QIcon::fromTheme\((QStringLiteral\()?"([^"]+)"\)?\).*/ s|.*QIcon::fromTheme\((QStringLiteral\()?"([^"]+)"\)?\).*|\2| p'`)
icons+=(
	subtitlecomposer
	waveform
	mediaplayer
	pocketsphinx
)
icons=(`(for i in "${icons[@]}" ; do echo $i ; done) | sort -u`)

iconfiles=()
for icon in "${icons[@]}"; do
	_icon="${icon/-/_}"
	file="$_rt/src/icons/sc-actions-$_icon.svg"
	[[ ! -f "$file" ]] && file="$(ls "$_rt/src/icons/"*"-actions-$_icon.png" 2>/dev/null | sort -n | head -n 1 || true)"
	[[ ! -f "$file" ]] && file="$(ls "$_rt/src/icons/"*"-categories-$_icon.png" 2>/dev/null | sort -n | head -n 1 || true)"
	[[ ! -f "$file" ]] && file="$(ls "$_rt/src/icons/"*"-apps-$_icon.png" 2>/dev/null | sort -n | head -n 1 || true)"
	[[ ! -f "$file" ]] && file="/usr/share/icons/breeze/actions/22/$icon.svg"
	[[ ! -f "$file" ]] && file="/usr/share/icons/breeze/actions/24/$icon.svg"
	[[ ! -f "$file" ]] && file="/usr/share/icons/breeze/status/22/$icon.svg"
	[[ ! -f "$file" ]] && file="/usr/share/icons/breeze/places/22/$icon.svg"
	[[ ! -f "$file" ]] && echo "MISSING: $file" 1>&2 && find /usr/share/icons/ -name "$icon*" -printf "\tCANDIDATE: %p\n" 1>&2 || iconfiles+=("\n<file alias=\"actions/22/$icon.${file##*.}\">$(echo "$file" | sed -Ee "s|$_rt/src/||")</file>")
done
echo '<!DOCTYPE RCC><RCC version="1.0">'
echo '<qresource prefix="/icons/breeze">'
echo -n '<file alias="index.theme">/usr/share/icons/breeze/index.theme</file>'
echo -e "${iconfiles[@]}" | sort
echo '</qresource>'
echo '</RCC>'
