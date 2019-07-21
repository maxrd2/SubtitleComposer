#!/bin/bash

set -e

_gitroot="$(cd $(dirname "$0") && echo $PWD)"
while [[ ! -d "$_gitroot/.git" ]]; do _gitroot="$(dirname "$_gitroot")" ; [[ "$_gitroot" == "/" ]] && echo "ERROR: cannot find .git directory" && exit 1 ; done
cd "$_gitroot"

sudo docker run --rm -v "$PWD":/home/devel -it maxrd2/arch-mingw /bin/bash -c 'bash pkg/mingw/build.sh'
