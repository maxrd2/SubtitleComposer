#!/bin/bash

set -e

_pr="$(readlink -f "$(dirname "$0")/../..")"
cd "$_pr"

appver="$(git describe --always --abbrev=8 | sed 's/-g/./;s/-/./;s/^v//g')"
rm -rf build &>/dev/null || true
mkdir -p build/nsis

sudo pacman -Sy --noconfirm --needed archlinux-keyring
sudo pacman -Su --noconfirm
sudo pacman -S --noconfirm --needed \
	mingw-w64-toolchain mingw-w64-cmake mingw-w64-configure mingw-w64-pkg-config \
	mingw-w64-ffmpeg mingw-w64-qt5 mingw-w64-kf5 nsis \
	mingw-w64-kinit mingw-w64-pango mingw-w64-libvisual \
	mingw-w64-aspell mingw-w64-hunspell mingw-w64-icu \
	mingw-w64-libidn2 mingw-w64-openal \
	kconfig kcoreaddons breeze-icons icu
sudo pacman -Sdd --noconfirm --needed kauth kbookmarks kcodecs kcompletion \
	kconfig kconfigwidgets kcoreaddons kglobalaccel kitemviews kjobwidgets \
	knotifications kross ktextwidgets kwidgetsaddons kwindowsystem kxmlgui \
	solid sonnet

i686-w64-mingw32-cmake -B build \
	-DCMAKE_BUILD_TYPE=Release \
	-DKDE_INSTALL_LIBDIR=lib \
	-DKDE_INSTALL_USE_QT_SYS_PATHS=ON \
	-DBUILD_TESTING=OFF \
	-DKF5_HOST_TOOLING=/usr/lib/cmake \
	-DKCONFIGCOMPILER_PATH=/usr/lib/cmake/KF5Config/KF5ConfigCompilerTargets.cmake \
	-DTARGETSFILE=/usr/lib/cmake/KF5CoreAddons/KF5CoreAddonsToolingTargets.cmake
cmake --build build -j$(nproc)
DESTDIR="$_pr/build/nsis" cmake --build build --target nsis
