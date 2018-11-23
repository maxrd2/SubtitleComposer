#!/bin/sh

cd "$(dirname "$0")"

export PATH="$PWD/usr/bin:$PATH"
export LD_LIBRARY_PATH="$PWD/usr/lib:$LD_LIBRARY_PATH"
export XDG_DATA_DIRS="$PWD/usr/share:$XDG_DATA_DIRS"
export XDG_CURRENT_DESKTOP=

exec "usr/bin/subtitlecomposer" "$@"
