#!/bin/bash

if [ "$1" == "" ]; then
	echo "Usage: copy.sh <all|git [commit]|files>"
	exit
fi

textc=-$(date +%Y)' Mladen Milinkovic <max\@smoothware.net>'
text1=':a;N;$!ba;s/\/\*[^\/]+SPDX-FileCopyrightText:([^\/]+|[^\*]\/)+\*\//\/*\n'
text2='\n    SPDX-License-Identifier: GPL-2.0-or-later\n*\//'

update_copyright() {
	f="$1"
	if [[ ! -f "$f" ]]; then
		return
	fi
	echo -e '\e[01;33mProcessing \e[01;39m'$f'\e[01;33m\e[00m'

	copy=$(perl -pe 'BEGIN{undef $/;} s!^.*?/\*[^/]*?(([\t *]*?SPDX-FileCopyrightText[^\n]*?[\t *]*?\n)+).*?$!$1!sg' "$f"\
		| perl -pe 's!\(([^)>]+@[^)>]+)\)!<$1>!' \
		| perl -pe "s!(-\d+)? Mladen Milinkovic [<(][^>]+[>)]!$textc!g" \
		| perl -pe 's!(\d{4})(-\1)!$1!g' \
		| perl -pe 'BEGIN{undef $/;} s!\n+!\\n!sg')

	if [[ $copy == *"Sergio Pistone"* || $copy == *"Mladen Milinkovic"* ]]; then
		if [[ $copy != *"Mladen Milinkovic"* ]]; then
			copy="$copy * Copyright (C) 2010-${textc:1}\n"
		fi
		sed -r "$text1$copy$text2" --in-place "$f"
	fi
}

if [ "$1" == "all" ]; then
	export -f update_copyright
	for f in `find . \( -name \*.cpp -or -name \*.h \)`; do
		update_copyright "$f"
	done
elif [ "$1" == "git" ]; then
	for f in `git diff --name-only "${@:2}" | grep -Pe '\.[ch](pp)?$'`; do
		update_copyright "$f"
	done
else
	for f in "${@:1}"; do
		update_copyright "$f"
	done
fi
