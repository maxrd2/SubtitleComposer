#!/bin/bash

set -e

# script tested on http://releases.ubuntu.com/14.04/ubuntu-14.04.5-server-amd64.iso

# setup
downloaddir="$HOME/download"
builddir="$HOME/build"
instdir="$HOME/dist"
appdir="$HOME/appdir"
pkgdir="$HOME/pkgdir"
export PATH="$instdir/usr/bin:$PATH"
export LD_LIBRARY_PATH="$instdir/usr/lib:$instdir/usr/lib/x86_64-linux-gnu:$LD_LIBRARY_PATH"
export CPATH="$instdir/usr/include:$CPATH"
export PKG_CONFIG_PATH="$instdir/usr/lib/pkgconfig:$PKG_CONFIG_PATH"
export XDG_DATA_DIRS="$instdir/usr/share:$XDG_DATA_DIRS"
mkdir -p "$instdir" "$builddir" "$downloaddir" "$pkgdir"
cd $builddir

download() {
	local filename=${1##*/}
	local destfile="$2"
	[[ -z "$destfile" ]] && destfile="$filename"
	if [[ ! -f "$downloaddir/$destfile" ]]; then
		echo -e "\033[93mDownloading $1...\033[m"
		curl -fL "$1" -o "$downloaddir/$destfile" || exit 1
	fi
}
sync_inst() {
	echo -e "\033[91mInstalling files to root...\033[m"
	sudo rsync -rlptD "$instdir/." /
	sudo ldconfig
}
pkg_build() {
	[[ -z "$1" ]] && echo -e "\033[91mERROR: package() needs a filename\033[m" && exit 1
	echo -e "\033[93mBuilding package '$1.tar.xz'...\033[m"
	(cd "$instdir" && tar -cJf "$pkgdir/$1.tar.xz" * && cd "$pkgdir" && sha256sum "$1.tar.xz" > "$1.tar.xz.sha256")
	sync_inst
	rm -rf "$instdir"/* "$builddir"/*
}
pkg_missing() {
	echo -e "\033[93mChecking package '$1.tar.xz'...\033[m"
	! [ -f "$pkgdir/$1.tar.xz" ]
}


if [[ ! -e "$HOME/apt_updated" ]]; then
	# update system
	sudo apt-get update
	sudo apt-get dist-upgrade -y
	touch "$HOME/apt_updated"
fi
	
# build requirements
sudo apt-get install -yqq build-essential bison autoconf curl wget cmake make git

# wayland
ver=1.16.0
pkg="wayland-$ver-trusty-amd64"

if pkg_missing $pkg; then
	sudo apt-get install -yqq libffi-dev libwayland-dev
	# hack: remove installed wayland package files, but keep package marked as installed :-/
	(sudo rm $(dpkg -L libwayland-client0 libwayland-cursor0 libwayland-server0 libwayland-dev) &>/dev/null || exit 0)
	file="wayland-$ver.tar.xz"
	download "https://wayland.freedesktop.org/releases/$file"
	tar -xJf "$downloaddir/$file" -C "$builddir"
	(cd wayland* \
		&& ./configure --prefix=/usr \
			--disable-documentation \
			--disable-static \
		&& make -j$(nproc) \
		&& make install DESTDIR="$instdir")
	pkg_build $pkg
fi

# ffmpeg
download "https://www.ffmpeg.org/releases/ffmpeg-snapshot-git.tar.bz2"
ver=$(date -r "$downloaddir/ffmpeg-snapshot-git.tar.bz2" +%Y%m%d)
pkg="ffmpeg-$ver-trusty-amd64"
if pkg_missing $pkg; then
	sudo apt-get install -yqq libass-dev libfreetype6-dev libsdl2-dev libtool libva-dev libvdpau-dev libvorbis-dev libxcb1-dev libxcb-shm0-dev libxcb-xfixes0-dev zlib1g-dev nasm pkgconf libglu1-mesa-dev libva-glx1 libglu1-mesa
	tar -xjf "$downloaddir/ffmpeg-snapshot-git.tar.bz2" ffmpeg
	(cd ffmpeg && ./configure --prefix=/usr --enable-libass --enable-libfreetype --enable-shared --disable-static --disable-doc && make -j$(nproc) && make install DESTDIR="$instdir") 
	pkg_build $pkg
fi

# libmpv
ver=0.29.1
pkg="libmpv-$ver-trusty-amd64"
if pkg_missing $pkg; then
	sudo apt-get install -yqq libgbm-dev
	download "https://github.com/mpv-player/mpv/archive/v$ver.tar.gz" mpv-$ver.tar.gz
	mkdir -p mpv ; tar -xzf "$downloaddir/mpv-$ver.tar.gz" -C mpv --strip-components=1
	(cd mpv && ./bootstrap.py && ./waf configure --prefix=/usr --enable-libmpv-shared --disable-cplayer --disable-libass && ./waf build -j$(nproc) && ./waf install --destdir="$instdir")
	pkg_build $pkg
fi

# sphinxbase
download "https://github.com/cmusphinx/sphinxbase/archive/master.tar.gz" sphinxbase-master.tar.gz
ver=$(date -r "$downloaddir/sphinxbase-master.tar.gz" +%Y%m%d)
pkg="sphinxbase-$ver-trusty-amd64"
if pkg_missing $pkg; then
	tar -xzf "$downloaddir/sphinxbase-master.tar.gz" -C "$builddir" sphinxbase-master
	NOCONFIGURE=1 "$builddir/sphinxbase-master/autogen.sh"
	(cd sphinxbase-master && ./configure --prefix=/usr --without-python && make -j$(nproc) && make install DESTDIR="$instdir")
	pkg_build $pkg
fi

# pocketsphinx
download "https://github.com/cmusphinx/pocketsphinx/archive/master.tar.gz" pocketsphinx-master.tar.gz
ver=$(date -r "$downloaddir/pocketsphinx-master.tar.gz" +%Y%m%d)
pkg="pocketsphinx-$ver-trusty-amd64"
if pkg_missing $pkg; then
	tar -xzf "$downloaddir/pocketsphinx-master.tar.gz" -C "$builddir" pocketsphinx-master
	NOCONFIGURE=1 "$builddir/pocketsphinx-master/autogen.sh"
	(cd pocketsphinx-master && ./configure --prefix=/usr --without-python && make -j$(nproc) && make install DESTDIR="$instdir")
	pkg_build $pkg
fi

# cmake
ver=3.11.0
pkg="cmake-$ver-trusty-amd64"
if pkg_missing $pkg; then
	download "https://cmake.org/files/v${ver%.*}/cmake-$ver-Linux-x86_64.tar.gz"
	mkdir -p "$instdir/usr"
	tar -xzf "$downloaddir/cmake-$ver-Linux-x86_64.tar.gz" -C "$instdir/usr" --strip-components=1
	pkg_build $pkg
fi

# qt5
ver=5.11.2
pkg="qt5-bundle-$ver-trusty-amd64"
if pkg_missing $pkg; then
	sudo apt-get install -yqq libfontconfig1-dev libfreetype6-dev libx11-dev libxext-dev libxfixes-dev libxi-dev libxrender-dev libxcb1-dev libx11-xcb-dev libxcb-glx0-dev \
		fontconfig libasound2-dev libatk1.0-dev libatspi2.0-dev libavahi-client-dev libavahi-common-dev libcairo2-dev libdbus-1-dev libdbus-glib-1-dev libdrm-dev \
		libgbm-dev libgcrypt11-dev libgl1-mesa-dev libgl1-mesa-glx libgles2-mesa-dev libglu1-mesa-dev libgnutls-dev libgpg-error-dev \
		libgstreamer-plugins-base0.10-dev libgstreamer0.10-dev libharfbuzz-dev libice-dev libjpeg-dev libjpeg-turbo8-dev libjpeg8-dev libkrb5-dev libltdl-dev \
		libopenvg1-mesa-dev libp11-kit-dev libpango1.0-dev libpixman-1-dev libpq-dev libprotobuf-dev libpulse-dev libsm-dev libxcb-xinerama0-dev \
		libssl-dev libtasn1-6-dev libudev-dev libwayland-dev libxcb-dri2-0-dev libxcb-icccm4-dev libxcb-image0-dev libxcb-keysyms1-dev libxcb-present-dev libxcb-randr0-dev \
		libxcb-render-util0-dev libxcb-render0-dev libxcb-shape0-dev libxcb-shm0-dev libxcb-sync-dev libxcb-xfixes0-dev libxcb-xkb-dev libxcomposite-dev libxcursor-dev \
		libxdamage-dev libxft-dev libxinerama-dev libxkbcommon-dev libxkbcommon-x11-dev libxrandr-dev libxshmfence-dev libxtst-dev libxxf86vm-dev mesa-common-dev \
		x11proto-composite-dev x11proto-damage-dev x11proto-dri2-dev x11proto-gl-dev x11proto-randr-dev x11proto-record-dev x11proto-xf86vidmode-dev x11proto-xinerama-dev \
		libegl1-mesa-drivers libglapi-mesa libicu-dev
		#libmirclient-dev libmirprotobuf-dev mircommon-dev libsqlite3-dev libegl1-mesa-dev
	build_qt() {
		local pkg="$1"
		local ver="$2"
		local verb=${ver%.*}
		local file="$pkg-everywhere-src-$ver.tar.xz"
		download "https://download.qt.io/archive/qt/$verb/$ver/submodules/$file"
		if [[ "$pkg" == "qtbase" ]]; then
			conf=(./configure -prefix /usr -opensource -confirm-license -nomake tests -nomake examples -dbus \
				-no-separate-debug-info -xcb -system-xcb -qpa xcb -no-eglfs -release -reduce-relocations \
				-optimized-qmake)
		else
			conf=(qmake)
		fi
		(mkdir -p $pkg && cd $pkg \
			&& echo -e "\033[93mExtracting $file...\033[m" \
			&& tar -xJpsf "$downloaddir/$file" --strip-components=1 \
			&& "${conf[@]}" \
			&& make -j$(nproc) \
			&& INSTALL_ROOT="$instdir" make install)
	}
	qt_modules=(qtbase qttools qtsvg qtx11extras qtscript qtxmlpatterns qtdeclarative qtimageformats qtquickcontrols qtquickcontrols2)
	for p in "${qt_modules[@]}" ; do 
		mod="qt5-$p-$ver-trusty-amd64"
		if pkg_missing $mod; then
			build_qt "$p" $ver
			pkg_build $mod
		fi
	done
	for p in "${qt_modules[@]}" ; do 
		mod="qt5-$p-$ver-trusty-amd64"
		tar -xJf "$pkgdir/$mod.tar.xz" -C "$instdir"
	done
	pkg_build $pkg
fi

# kf5
ver=5.52.0
pkg="kf5-bundle-$ver-trusty-amd64"
if pkg_missing $pkg; then
	sudo apt-get install -yqq gperf flex libattr1-dev libcanberra-dev libxslt1-dev libboost1.55-all-dev
	
	build_kf5() {
		local pkg="$1"
		local ver="$2"
		local verb=${ver%.*}
		local file="$pkg-$ver.tar.xz"
		download "https://download.kde.org/stable/frameworks/$verb/$file"
		(mkdir -p $pkg/build && cd $pkg \
			&& echo -e "Extracting $file..." \
			&& tar -xJpsf "$downloaddir/$file" --strip-components=1 \
			&& cd build \
			&& cmake \
				-DCMAKE_INSTALL_PREFIX=/usr \
				-DCMAKE_BUILD_TYPE=Release \
				-DBUILD_TESTING=OFF \
				.. \
			&& make -j$(nproc) \
			&& make install DESTDIR="$instdir")
	}
	
	kf5_modules=(
		extra-cmake-modules
		# tier 1
		attica kconfig kapidox kdnssd kidletime kplotting kguiaddons ki18n kitemviews sonnet kwidgetsaddons kwindowsystem 
		kdbusaddons karchive kcoreaddons kcodecs solid kitemmodels threadweaver syntax-highlighting breeze-icons kwayland
		#bluez-qt modemmanager-qt networkmanager-qt prison
		# tier 2
		kcompletion kfilemetadata kjobwidgets kcrash kimageformats kunitconversion kauth knotifications kpackage kpty
		#kdoctools 
		# tier 3
		kservice kdesu kemoticons kpeople kconfigwidgets kiconthemes ktextwidgets kglobalaccel kxmlgui kbookmarks
		kwallet kio kactivities kxmlrpcclient kparts kdesignerplugin knewstuff ktexteditor kdeclarative kirigami2
		plasma-framework kcmutils knotifyconfig krunner kinit
		#kactivities-stats baloo kded kdewebkit
	)
	for p in "${kf5_modules[@]}" ; do
		mod="kf5-$p-$ver-trusty-amd64"
		if pkg_missing $mod; then
			build_kf5 "$p" $ver
			pkg_build $mod
		fi
	done
	for p in "${kf5_modules[@]}" ; do 
		mod="kf5-$p-$ver-trusty-amd64"
		tar -xJf "$pkgdir/$mod.tar.xz" -C "$instdir"
	done
	pkg_build $pkg
fi

ver=5.14.4
pkg="plasma-bundle-$ver-trusty-amd64"
if pkg_missing $pkg; then
	sudo apt-get install -yqq libfftw3-dev

	build_plasma() {
		local pkg="$1"
		local ver="$2"
		local file="$pkg-$ver.tar.xz"
		download "https://download.kde.org/stable/plasma/$ver/$pkg-$ver.tar.xz"
		(mkdir -p $pkg/build && cd $pkg \
			&& echo -e "Extracting $file..." \
			&& tar -xJpsf "$downloaddir/$file" --strip-components=1 \
			&& cd build \
			&& cmake \
				-DCMAKE_INSTALL_PREFIX=/usr \
				-DCMAKE_BUILD_TYPE=Release \
				-DBUILD_TESTING=OFF \
				.. \
			&& make -j$(nproc) \
			&& make install DESTDIR="$instdir")
	}
	plasma_modules=(kdecoration breeze plasma-integration)
	for p in "${plasma_modules[@]}" ; do
		mod="plasma-$p-$ver-trusty-amd64"
		if pkg_missing $mod; then
			build_plasma "$p" $ver
			pkg_build $mod
		fi
	done
	for p in "${plasma_modules[@]}" ; do
		mod="plasma-$p-$ver-trusty-amd64"
		tar -xJf "$pkgdir/$mod.tar.xz" -C "$instdir"
	done
	pkg_build $pkg
fi

# kross
ver=5.52.0
pkg="kross-$ver-trusty-amd64"
if pkg_missing $pkg; then
	file="kross-$ver.tar.xz"
		download "https://download.kde.org/stable/frameworks/${ver%.*}/portingAids/$file"
	(mkdir -p $pkg/build && cd $pkg \
		&& echo -e "Extracting $file..." \
		&& tar -xJpsf "$downloaddir/$file" --strip-components=1 \
		&& cd build \
		&& cmake \
			-DCMAKE_PREFIX_PATH="$instdir" \
			-DCMAKE_INSTALL_PREFIX=/usr \
			-DCMAKE_BUILD_TYPE=Release \
			-DBUILD_TESTING=OFF \
			.. \
		&& make -j$(nproc) \
		&& make install DESTDIR="$instdir")
	pkg_build $pkg
fi

# kross-interpreters
ver=18.08.3
pkg="kross-interpreters-$ver-trusty-amd64"
if pkg_missing $pkg; then
	file="kross-interpreters-$ver.tar.xz"
	download "https://download.kde.org/stable/applications/$ver/src/$file"
	(mkdir -p $pkg/build && cd $pkg \
		&& echo -e "Extracting $file..." \
		&& tar -xJpsf "$downloaddir/$file" --strip-components=1 \
		&& cd build \
		&& cmake \
			-DCMAKE_PREFIX_PATH="$instdir" \
			-DCMAKE_INSTALL_PREFIX=/usr \
			-DCMAKE_BUILD_TYPE=Release \
			-DBUILD_TESTING=OFF \
			.. \
		&& make -j$(nproc) \
		&& make install DESTDIR="$instdir")
	pkg_build $pkg
fi

echo -e "\033[93mBuilding subtitlecomposer appimage...\033[m"
# subtitlecomposer requirements
sudo apt-get install -yqq libicu-dev zsync
# gstreamer module requirements
#sudo apt-get install -yqq libgstreamer1.0-dev libgstreamer-plugins-good1.0-dev libgstreamer-plugins-base1.0-dev
# phonon module requirements - it doesn't work that good will skip it
#sudo apt-get install -yqq libphonon4qt5-dev libphonon4qt5experimental-dev
# xine module requirements - requires libavformat, will skip building this plugin
# sudo apt-get install -yqq libxine2-dev

# appdir
#git clone --branch travis --depth 1 https://github.com/maxrd2/subtitlecomposer.git "$builddir/subtitlecomposer"
git clone --branch travis --depth 1 ssh://max@beeblebrox/home/max/projects/SubtitleComposer "$builddir/subtitlecomposer"
mkdir -p "$builddir/subtitlecomposer/build"
(cd "$builddir/subtitlecomposer/build" \
	&& cmake \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_INSTALL_PREFIX=/usr \
		-DKDE_INSTALL_LIBDIR=lib \
		-DKDE_INSTALL_USE_QT_SYS_PATHS=ON \
		-DBUILD_TESTING=OFF \
		.. \
	&& make -j$(nproc) \
	&& make install DESTDIR="$appdir")

# pocketsphinx
mkdir -p "$appdir/usr/share/pocketsphinx"
cp -rf /usr/share/pocketsphinx/model "$appdir/usr/share/pocketsphinx/"

# add kio
tar -xJf "$pkgdir/kf5-kio-5.52.0-trusty-amd64.tar.xz" -C "$appdir/"

# add kross
tar -xJf "$pkgdir/kross-5.52.0-trusty-amd64.tar.xz" -C "$appdir/"
tar -xJf "$pkgdir/kross-interpreters-18.08.3-trusty-amd64.tar.xz" -C "$appdir/"
[ -f "$appdir/usr/plugins/krossruby.so" ] && rm "$appdir/usr/plugins/krossruby.so"
rm "$appdir/usr/share/subtitlecomposer/scripts"/*.rb

# icons
tar -xJf "$pkgdir/kf5-breeze-icons-5.52.0-trusty-amd64.tar.xz" -C "$appdir/"

# style
tar -xJf "$pkgdir/plasma-breeze-5.14.4-trusty-amd64.tar.xz" -C "$appdir/"

# relocate x64 binaries from arch specific directory as required for appimages
[ -d "$appdir/usr/lib/x86_64-linux-gnu" ] && mv "$appdir/usr/lib/x86_64-linux-gnu"/* "$appdir/usr/lib" && rm -rf "$appdir/usr/lib/x86_64-linux-gnu"
[ -d "$appdir/usr/lib/libexec" ] && mv "$appdir/usr/lib/libexec" "$appdir/usr/libexec" && mv "$appdir/usr/libexec/kf5"/* "$appdir/usr/libexec" && rmdir "$appdir/usr/libexec/kf5"

# delete useless stuff
rm -rf "$appdir/usr/share/wallpapers"
rm -rf "$appdir/usr/share/icons/"{breeze-dark,Breeze_Snow,breeze_cursors,breeze/breeze-icons.rcc}
rm -rf "$appdir/usr/qml"
rm -rf "$appdir/usr/lib/cmake"
rm -rf "$appdir/usr/include"
rm -rf "$appdir/usr/mkspecs/"

# appimage
cd "$appdir/.."
download "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage" linuxdeployqt
download "https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage" appimagetool
chmod +x "$downloaddir/linuxdeployqt" "$downloaddir/appimagetool"

execs=("$appdir/usr/lib/subtitlecomposer/"*.so)
#execs+=("$appdir/usr/plugins/styles/"*.so "$appdir/usr/plugins/iconengines/"*.so)
#plugins=("$appdir/usr/plugins/kf5/kio/"*.so) ; mkdir -p "$appdir/usr/plugins/kf5/kio/" ; cp -ax /usr/plugins/kf5/kio/*.so "$appdir/usr/plugins/kf5/kio/"
"$downloaddir/linuxdeployqt" "$appdir/usr/share/applications/subtitlecomposer.desktop" -bundle-non-qt-libs "${execs[@]/#/-executable=}" #-extra-plugins="$(IFS=','; echo "${plugins[*]}")"

rm -v "$appdir/AppRun" && cp -v "$builddir/subtitlecomposer/pkg/misc/subtitlecomposer.sh" "$appdir/AppRun" && chmod +x "$appdir/AppRun"
rm -v "$appdir/usr/lib/libxcb-dri2.so"* "$appdir/usr/lib/libxcb-dri3.so"*

"$downloaddir/appimagetool" "$appdir/" -u 'gh-releases-zsync|maxrd2|subtitlecomposer|continuous|Subtitle_Composer-*x86_64.AppImage.zsync'

#[ ! -f Subtitle_Composer*.AppImage.zsync ] && zsyncmake Subtitle_Composer*.AppImage
#mv Subtitle_Composer*.AppImage* "$pkgdir/"

# remove build dir
rm -rf "$builddir/subtitlecomposer"
