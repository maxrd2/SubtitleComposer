#!/bin/bash

if [ "$1" == "" ]; then
	echo "Usage: copy.sh <commit>"
	exit
fi

textc=-$(date +%Y)' Mladen Milinkovic <max\@smoothware.net>'
text1=':a;N;$!ba;s/\/\*[^\/]+Copyright([^\/]+|[^\*]\/)+\*\//\/*\n'
text2=' *\
 * This program is free software; you can redistribute it and\/or modify\
 * it under the terms of the GNU General Public License as published by\
 * the Free Software Foundation; either version 2 of the License, or\
 * (at your option) any later version.\
 *\
 * This program is distributed in the hope that it will be useful,\
 * but WITHOUT ANY WARRANTY; without even the implied warranty of\
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\
 * GNU General Public License for more details.\
 *\
 * You should have received a copy of the GNU General Public License\
 * along with this program; if not, write to the\
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,\
 * Boston, MA 02110-1301, USA.\
 *\//'

for f in `git diff --name-only "$@" | grep -Pe '\.[ch](pp)?$'` ; do
	if [[ ! -f "$f" ]]; then
		continue
	fi
	echo -e '\e[01;33mProcessing \e[01;39m'$f'\e[01;33m\e[00m'
	copy=$(perl -pe 'BEGIN{undef $/;} s!^.*?/\*[^/]*?(([\t *]*?Copyright[^\n]*?[\t *]*?\n)+).*?$!$1!sg' "$f" \
		| perl -pe 's!(^|\n)[\t *]+! * !mg' \
		| perl -pe 's![\t *]+$!!mg' \
		| perl -pe 's!\(([^)>]+@[^)>]+)\)!<$1>!' \
		| perl -pe "s!(-\d+)? Mladen Milinkovic [<(][^>]+[>)]!$textc!g" \
		| perl -pe 'BEGIN{undef $/;} s!\n+!\\n!sg')

	if [[ $copy == *"Sergio Pistone"* || $copy == *"Mladen Milinkovic"* ]]; then
		if [[ $copy != *"Mladen Milinkovic"* ]]; then
			copy="$copy * Copyright (C) 2010-${textc:1}\n"
		fi
		sed -r "$text1$copy$text2" --in-place "$f"
	fi
done

