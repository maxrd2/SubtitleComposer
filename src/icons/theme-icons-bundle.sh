#!/bin/sh

set -e

icon_src_dir="$(dirname "$(readlink -f "$0")")"

icons=(`grep -hr QIcon::fromTheme "$icon_src_dir/../../src" \
	| sed -n -E -e '/.*QIcon::fromTheme\((QStringLiteral\()?"([^"]+)"\)?\).*/ s|.*QIcon::fromTheme\((QStringLiteral\()?"([^"]+)"\)?\).*|\2| p'`)
icons+=(
	kde
	mail-send
	languages
	set-language
	preferences-desktop-locale
	dialog-cancel
	dialog-close
	dialog-input-devices
	dialog-messages
	dialog-ok-apply
	dialog-ok
	dialog-scripts
	dialog-xml-editor
	edit-clear-all
	edit-clear-history
	edit-clear-list
	edit-clear-locationbar-ltr
	edit-clear-locationbar-rtl
	edit-clear
	edit-clone
	edit-comment
	edit-copy
	edit-cut
	edit-delete-remove
	edit-delete-shred
	edit-delete
	edit-download
	edit-entry
	edit-find
	edit-none
	edit-paste
	edit-redo
	edit-rename
	edit-reset
	edit-select-all
	edit-select-invert
	edit-select-none
	edit-select
	edit-select-text
	edit-undo
	tools-check-spelling
	tools-report-bug
	tools-wizard
	settings-configure
	configure
	configure-shortcuts
	configure-toolbars
	help-about
	help-contents
	help-contextual
	help-donate
	help-feedback
	help-hint
	help-keybord-shortcuts
	help-latex
	help-whatsthis
)
icons=(`(for i in "${icons[@]}" ; do echo $i ; done) | sort -u`)

theme_dir="$1"
[ -z "$theme_dir" -o ! -d "$theme_dir" ] && theme_dir="/usr/share/icons/breeze"

qrcEntries=(
#	"<file alias=\"breeze/index.theme\">$theme_dir/index.theme</file>\n"
	"<file alias=\"breeze/index.theme\">$icon_src_dir/breeze.theme</file>\n"
)
groups=(actions/22 actions/24 status/22 places/22 apps/24 apps/22 preferences/32)
for icon in "${icons[@]}"; do
	_icon="${icon/-/_}"

	# check if already bundle
	grep -q -E "(\"|/)$icon.(png|svg)\"" "$icon_src_dir/icon-bundle.qrc" \
		&& echo -e "\t\e[32m'$icon'\e[m already in 'icon-bundle.qrc'" 1>&2 && continue

	# icons in different dirs are different, thus not using *
	for grp in "${groups[@]}"; do
		file="$theme_dir/$grp/$icon.svg"
		[ -f "$file" ] && break
	done

	if [ ! -f "$file" ]; then
		echo -e "\t\e[1;33mWARNING: can't find '$icon'\e[m" 1>&2
		find "$theme_dir" -name "$icon*" -printf "\t- candidate: %p\n" 1>&2
		continue
	fi

	# finally bundle the icon
	_alias="$icon.${file##*.}"
	_file="$(echo "$file" | sed -Ee "s|$_rt/src/||")"

	echo -e "\t'$_alias' => '$_file'" 1>&2
	qrcEntries+=("<file alias=\"breeze/actions/22/$_alias\">$_file</file>\n")
done

# generate .qrc file
echo '<!DOCTYPE RCC>'
echo '<RCC version="1.0">'
echo '<qresource prefix="/icons">'
echo -en "${qrcEntries[@]}"
echo '</qresource>'
echo '</RCC>'
