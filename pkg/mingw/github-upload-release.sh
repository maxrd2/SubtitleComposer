#!/bin/bash

# Build
build=pkg/mingw/docker-build.sh
echo -e "Starting docker build '\e[1;39m$build\e[m'..."
$build

pkg/misc/github-release.sh win32 "$PWD/build/SubtitleComposerSetup.exe"
