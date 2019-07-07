#!/bin/bash

set -e

_gitroot="$(cd $(dirname "$0") && echo $PWD)"
while [[ ! -d "$_gitroot/.git" ]]; do _gitroot="$(dirname "$_gitroot")" ; [[ "$_gitroot" == "/" ]] && echo "ERROR: cannot find .git directory" && exit 1 ; done
cd "$_gitroot"

appver="$(git describe --always --abbrev=8 | sed 's/-g/./;s/-/./;s/^v//g')"
rm -rf build && mkdir -p build/nsis

sudo docker run --rm -v "$PWD":/home/devel -it maxrd2/arch-mingw /bin/bash -c '\
	sudo pacman -Sy --noconfirm --needed \
		mingw-w64-kinit mingw-w64-pango mingw-w64-libvisual \
		mingw-w64-aspell mingw-w64-hunspell mingw-w64-icu \
		kconfig kcoreaddons breeze-icons \
		mingw-w64-mpv \
		mingw-w64-gst-libav mingw-w64-gst-plugins-good && \
	cd build && \
	i686-w64-mingw32-cmake \
		-DCMAKE_BUILD_TYPE=Release \
		-DKDE_INSTALL_LIBDIR=lib \
		-DKDE_INSTALL_USE_QT_SYS_PATHS=ON \
		-DBUILD_TESTING=OFF \
		-DKF5_HOST_TOOLING=/usr/lib/cmake \
		-DKCONFIGCOMPILER_PATH=/usr/lib/cmake/KF5Config/KF5ConfigCompilerTargets.cmake \
		-DTARGETSFILE=/usr/lib/cmake/KF5CoreAddons/KF5CoreAddonsToolingTargets.cmake \
		-DAPP_VERSION='$appver' \
		.. && \
	make -j$(nproc) && \
	make DESTDIR="$PWD/nsis" nsis'
