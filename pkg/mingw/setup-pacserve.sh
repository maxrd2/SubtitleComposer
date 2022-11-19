#!/bin/sh

set -e

[ -z "$1" ] && { echo "Usage: setup-cache.sh <pacserve ip>" 1>&2 ; exit 1 ; }
[ "$UID" == "0" ] || { echo "Run me as root" 1>&2 ; exit 1 ; }

use_cache="$1"
sed -r -e '1 i Server = http://'$use_cache':15678/pacman/$repo/$arch' -i /etc/pacman.d/mirrorlist
sed -r -e '/^Server\s*=/ i Server = http://'$use_cache':15678/pacman/$repo' -i /etc/pacman.conf
