#!/bin/bash

_rt="$(cd "$(dirname "$0")/../.." && pwd)"

icons=(`grep -hr QIcon::fromTheme "$_rt/src" \
	| sed -n -E -e '/.*QIcon::fromTheme\((QStringLiteral\()?"([^"]+)"\)?\).*/ s|.*QIcon::fromTheme\((QStringLiteral\()?"([^"]+)"\)?\).*|\2| p' \
	| sort -u`)

echo "${icons[@]}"
