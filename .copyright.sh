#!/bin/bash

if [ "$1" == "" ]; then
	echo "Usage: copy.sh <commit>"
	exit
fi

TEXT=':a;N;$!ba;s/\/\*[^\/]+Copyright([^\/]+|[^\*]\/)+\*\//\/*\
 * Copyright (C) 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>\
 * Copyright (C) 2010-2017 Mladen Milinkovic <max@smoothware.net>\
 *\
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

 
for f in `git diff --name-only "$@"` ; do
	if [[ ! -f "$f" ]]; then
		continue
	fi
	echo -e '\e[01;33mProcessing \e[01;39m'$f'\e[01;33m\e[00m'
	grep -n Copyright "$f" | grep -v Mladen | grep -v Sergio
	sed -r "$TEXT" --in-place "$f"
done
	
