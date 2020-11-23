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

pkg_patch() {
	case "$1" in
	"qtbase")
		# fix system xcb header
		sudo sed -e 's|\btemplate\b|tpl|' -i /usr/include/xcb/xcb_renderutil.h
		;;

	"kcoreaddons")
		patch -p1 <<EOF
diff --git a/src/desktoptojson/main.cpp b/src/desktoptojson/main.cpp
index 33190ae..17da30c 100644
--- a/src/desktoptojson/main.cpp
+++ b/src/desktoptojson/main.cpp
@@ -10,7 +10,7 @@

 static void messageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
 {
-    auto getFprintfS = [](auto data) {
+    auto getFprintfS = [](const char *data) {
         if (data == nullptr)
             return "";
         return data;
EOF
		;;

	"kimageformats")
		patch -p1 <<EOF
diff --git a/src/imageformats/xcf.cpp b/src/imageformats/xcf.cpp
index c508063..d696106 100644
--- a/src/imageformats/xcf.cpp
+++ b/src/imageformats/xcf.cpp
@@ -30,7 +30,7 @@ const float INCHESPERMETER = (100.0f / 2.54f);
 namespace {
 struct RandomTable {
     // From glibc
-    static constexpr int rand_r (unsigned int *seed)
+    static int rand_r (unsigned int *seed)
     {
         unsigned int next = *seed;
         int result = 0;
@@ -54,7 +54,7 @@ struct RandomTable {
         return result;
     }

-    constexpr RandomTable() : values{}
+    RandomTable() : values{}
     {
         unsigned int next = RANDOM_SEED;

@@ -370,7 +370,7 @@ private:
     static int random_table[RANDOM_TABLE_SIZE];
     static bool random_table_initialized;

-    static constexpr RandomTable randomTable{};
+    static RandomTable randomTable;

     //! This table is used as a shared grayscale ramp to be set on grayscale
     //! images. This is because Qt does not differentiate between indexed and
@@ -451,7 +451,7 @@ private:
 int XCFImageFormat::random_table[RANDOM_TABLE_SIZE];
 bool XCFImageFormat::random_table_initialized;

-constexpr RandomTable XCFImageFormat::randomTable;
+RandomTable XCFImageFormat::randomTable{};

 QVector<QRgb> XCFImageFormat::grayTable;
EOF
		;;

	"kxmlgui")
		patch -p1 <<EOF
diff --git a/src/kcheckaccelerators.cpp b/src/kcheckaccelerators.cpp
index de235cb..4caf940 100644
--- a/src/kcheckaccelerators.cpp
+++ b/src/kcheckaccelerators.cpp
@@ -265,7 +265,7 @@ void KCheckAccelerators::createDialog(QWidget *actWin, bool automatic)
     }
     QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, drklash);
     layout->addWidget(buttonBox);
-    connect(buttonBox, &QDialogButtonBox::rejected, drklash, &QDialog::close);
+    connect(buttonBox, &QDialogButtonBox::rejected, drklash.data(), &QDialog::close);
     if (disableAutoCheck) {
         disableAutoCheck->setFocus();
     } else {
diff --git a/src/kxmlguiwindow.cpp b/src/kxmlguiwindow.cpp
index ef1223b..5c77178 100644
--- a/src/kxmlguiwindow.cpp
+++ b/src/kxmlguiwindow.cpp
@@ -151,7 +151,7 @@ void KXmlGuiWindow::configureToolbars()
     if (!d->toolBarEditor) {
         d->toolBarEditor = new KEditToolBar(guiFactory(), this);
         d->toolBarEditor->setAttribute(Qt::WA_DeleteOnClose);
-        connect(d->toolBarEditor, &KEditToolBar::newToolBarConfig,
+        connect(d->toolBarEditor.data(), &KEditToolBar::newToolBarConfig,
                 this, &KXmlGuiWindow::saveNewToolbarConfig);
     }
     d->toolBarEditor->show();
EOF
		;;

	"kio")
		patch -p1 <<EOF
diff --git a/src/core/multigetjob.cpp b/src/core/multigetjob.cpp
index 9796947..fd56aeb 100644
--- a/src/core/multigetjob.cpp
+++ b/src/core/multigetjob.cpp
@@ -216,7 +216,9 @@ void MultiGetJob::slotMimetype(const QString &_mimetype)
         MultiGetJobPrivate::RequestQueue newQueue;
         d->flushQueue(newQueue);
         if (!newQueue.empty()) {
-            d->m_activeQueue.splice(d->m_activeQueue.cend(), newQueue);
+            auto it = newQueue.begin();
+            while(it != newQueue.end())
+                d->m_activeQueue.push_back(std::move(*it++));
             d->m_slave->send(d->m_command, d->m_packedArgs);
         }
     }
diff --git a/src/core/transferjob.cpp b/src/core/transferjob.cpp
index a310846..4919bc8 100644
--- a/src/core/transferjob.cpp
+++ b/src/core/transferjob.cpp
@@ -303,14 +303,14 @@ void TransferJobPrivate::start(Slave *slave)

     if (m_outgoingDataSource) {
         if (m_extraFlags & JobPrivate::EF_TransferJobAsync) {
-            q->connect(m_outgoingDataSource, &QIODevice::readyRead, q, [this]() {
+            q->connect(m_outgoingDataSource.data(), &QIODevice::readyRead, q, [this]() {
                 slotDataReqFromDevice();
             });
-            q->connect(m_outgoingDataSource, SIGNAL(readChannelFinished()),
+            q->connect(m_outgoingDataSource.data(), SIGNAL(readChannelFinished()),
                     SLOT(slotIODeviceClosed()));
             // We don't really need to disconnect since we're never checking
             // m_closedBeforeStart again but it's the proper thing to do logically
-            QObject::disconnect(m_outgoingDataSource, SIGNAL(readChannelFinished()),
+            QObject::disconnect(m_outgoingDataSource.data(), SIGNAL(readChannelFinished()),
                                 q, SLOT(slotIODeviceClosedBeforeStart()));
             if (m_closedBeforeStart) {
                 QMetaObject::invokeMethod(q, "slotIODeviceClosed", Qt::QueuedConnection);
diff --git a/src/gui/commandlauncherjob.cpp b/src/gui/commandlauncherjob.cpp
index ebe16ea..a7d6d93 100644
--- a/src/gui/commandlauncherjob.cpp
+++ b/src/gui/commandlauncherjob.cpp
@@ -91,12 +91,12 @@ void KIO::CommandLauncherJob::start()
     d->m_processRunner = KProcessRunner::fromCommand(d->m_command, d->m_desktopName, d->m_executable,
                                                      d->m_iconName, d->m_startupId,
                                                      d->m_workingDirectory);
-    connect(d->m_processRunner, &KProcessRunner::error, this, [this](const QString &errorText) {
+    connect(d->m_processRunner.data(), &KProcessRunner::error, this, [this](const QString &errorText) {
         setError(KJob::UserDefinedError);
         setErrorText(errorText);
         emitResult();
     });
-    connect(d->m_processRunner, &KProcessRunner::processStarted, this, [this](qint64 pid) {
+    connect(d->m_processRunner.data(), &KProcessRunner::processStarted, this, [this](qint64 pid) {
         d->m_pid = pid;
         emitResult();
     });
diff --git a/src/widgets/previewjob.cpp b/src/widgets/previewjob.cpp
index e025eb7..76e29a8 100644
--- a/src/widgets/previewjob.cpp
+++ b/src/widgets/previewjob.cpp
@@ -393,8 +393,8 @@ void PreviewJob::removeItem(const QUrl &url)
 {
     Q_D(PreviewJob);

-    auto it = d->items.cbegin();
-    while (it != d->items.cend()) {
+    auto it = d->items.begin();
+    while (it != d->items.end()) {
         if ((*it).item.url() == url) {
             d->items.erase(it);
             break;
diff --git a/src/widgets/renamedialog.cpp b/src/widgets/renamedialog.cpp
index 023c6a7..bbcfeaf 100644
--- a/src/widgets/renamedialog.cpp
+++ b/src/widgets/renamedialog.cpp
@@ -400,16 +400,16 @@ RenameDialog::RenameDialog(QWidget *parent, const QString &_caption,
         // check files contents for local files
         if (d->dest.isLocalFile() && d->src.isLocalFile()) {

-            const CompareFilesResult CompareFilesResult = compareFiles(d->src.toLocalFile(), d->dest.toLocalFile());
+            const CompareFilesResult compareFilesResult = compareFiles(d->src.toLocalFile(), d->dest.toLocalFile());

             QString text;
-            switch (CompareFilesResult) {
+            switch (compareFilesResult) {
                 case CompareFilesResult::Identical: text = i18n("The files are identical."); break;
                 case CompareFilesResult::PartiallyIdentical: text = i18n("The files seem identical."); break;
                 case CompareFilesResult::Different: text = i18n("The files are different."); break;
             }
             QLabel* filesIdenticalLabel = createLabel(this, text, true);
-            if (CompareFilesResult == CompareFilesResult::PartiallyIdentical) {
+            if (compareFilesResult == CompareFilesResult::PartiallyIdentical) {
                 QLabel* pixmapLabel = new QLabel(this);
                 pixmapLabel->setPixmap(QIcon::fromTheme(QStringLiteral("help-about")).pixmap(QSize(16,16)));
                 pixmapLabel->setToolTip(
diff --git a/src/filewidgets/kfilewidget.cpp b/src/filewidgets/kfilewidget.cpp
index f2ed01b..23f89fe 100644
--- a/src/filewidgets/kfilewidget.cpp
+++ b/src/filewidgets/kfilewidget.cpp
@@ -286,7 +286,7 @@ public:
     bool differentHierarchyLevelItemsEntered;

     const std::array<KIconLoader::StdSizes, 6> stdIconSizes =
-            {KIconLoader::SizeSmall, KIconLoader::SizeSmallMedium, KIconLoader::SizeMedium,
+            std::array<KIconLoader::StdSizes, 6>{KIconLoader::SizeSmall, KIconLoader::SizeSmallMedium, KIconLoader::SizeMedium,
              KIconLoader::SizeLarge, KIconLoader::SizeHuge, KIconLoader::SizeEnormous};

     QSlider *iconSizeSlider;
diff --git a/src/filewidgets/kfileplacesview.cpp b/src/filewidgets/kfileplacesview.cpp
index 0953624..498f8d2 100644
--- a/src/filewidgets/kfileplacesview.cpp
+++ b/src/filewidgets/kfileplacesview.cpp
@@ -244,7 +244,7 @@ void KFilePlacesViewDelegate::paint(QPainter *painter, const QStyleOptionViewIte
             if (!info.job &&
                     (!info.lastUpdated.isValid() || info.lastUpdated.secsTo(QDateTime::currentDateTimeUtc()) > 60)) {
                 info.job = KIO::fileSystemFreeSpace(url);
-                connect(info.job, &KIO::FileSystemFreeSpaceJob::result, this, [this, persistentIndex](KIO::Job *job, KIO::filesize_t size, KIO::filesize_t available) {
+                connect(info.job.data(), &KIO::FileSystemFreeSpaceJob::result, this, [this, persistentIndex](KIO::Job *job, KIO::filesize_t size, KIO::filesize_t available) {
                     PlaceFreeSpaceInfo &info = m_freeSpaceInfo[persistentIndex];

                     // even if we receive an error we want to refresh lastUpdated to avoid repeatedly querying in this case
EOF
		;;

	"breeze")
		sed -E -e 's|\bconstexpr\b||g' -i kstyle/breezehelper.h
		;;

	"plasma-integration")
		patch -p1 <<EOF
diff --git a/src/platformtheme/kdeplatformsystemtrayicon.cpp b/src/platformtheme/kdeplatformsystemtrayicon.cpp
index 33ca0db..e69c645 100644
--- a/src/platformtheme/kdeplatformsystemtrayicon.cpp
+++ b/src/platformtheme/kdeplatformsystemtrayicon.cpp
@@ -174,8 +174,8 @@ QMenu *SystemTrayMenu::menu()
 void SystemTrayMenu::createMenu()
 {
     m_menu = new QMenu();
-    connect(m_menu, &QMenu::aboutToShow, this, &QPlatformMenu::aboutToShow);
-    connect(m_menu, &QMenu::aboutToHide, this, &QPlatformMenu::aboutToHide);
+    connect(m_menu.data(), &QMenu::aboutToShow, this, &QPlatformMenu::aboutToShow);
+    connect(m_menu.data(), &QMenu::aboutToHide, this, &QPlatformMenu::aboutToHide);

     if (!m_icon.isNull()) {
         m_menu->setIcon(m_icon);
EOF
		;;
	esac

	return 0
}



if [[ ! -e "$HOME/apt_updated" ]]; then
	# update system
	sudo apt-get update
	sudo apt-get dist-upgrade -y
	touch "$HOME/apt_updated"
fi
	
# build requirements
sudo apt-get install -yqq build-essential bison autoconf curl wget cmake make git

# cmake
ver=3.19.0
pkg="cmake-$ver-trusty-amd64"
if pkg_missing $pkg; then
	download "https://cmake.org/files/v${ver%.*}/cmake-$ver-Linux-x86_64.tar.gz"
	mkdir -p "$instdir/usr"
	tar -xzf "$downloaddir/cmake-$ver-Linux-x86_64.tar.gz" -C "$instdir/usr" --strip-components=1
	pkg_build $pkg
fi

# meson, ninja
if [[ ! -x /usr/local/bin/meson ]]; then
	sudo apt-get install -yqq python3.5 python3-pip
	sudo python3.5 -m pip install --upgrade pip
	sudo pip install meson ninja
fi

# nasm
ver=2.15.04
download "https://www.nasm.us/pub/nasm/releasebuilds/${ver}/nasm-${ver}.tar.xz"
pkg="nasm-$ver-trusty-amd64"
if pkg_missing $pkg; then
# 	sudo apt-get install -yqq yasm
	tar -xJf "$downloaddir/nasm-${ver}.tar.xz" nasm-${ver}
	(cd nasm-${ver} \
		&& ./configure --prefix=/usr \
		&& make -j$(nproc) && make install DESTDIR="$instdir")
	pkg_build $pkg
fi

# wayland
ver=1.18.0
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

# wayland-protocols
ver=1.20
pkg="wayland-protocols-$ver-trusty-amd64"
if pkg_missing $pkg; then
	(sudo rm $(dpkg -L libwayland-client0 libwayland-cursor0 libwayland-server0 libwayland-dev) &>/dev/null || exit 0)
	file="wayland-protocols-$ver.tar.xz"
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

# xkbcommon
ver=1.0.3
download "https://github.com/xkbcommon/libxkbcommon/archive/xkbcommon-${ver}.tar.gz"
pkg="xkbcommon-$ver-trusty-amd64"
if pkg_missing $pkg; then
	sudo apt-get install -yqq libxkbcommon-dev libxkbcommon-x11-dev
	# hack: remove installed package files, but keep package marked as installed :-/
	(sudo rm $(dpkg -L libxkbcommon-dev libxkbcommon-x11-dev) &>/dev/null || exit 0)
	tar -xzf "$downloaddir/xkbcommon-$ver.tar.gz" "libxkbcommon-xkbcommon-${ver}"
	(cd "libxkbcommon-xkbcommon-${ver}" \
		&& meson setup --prefix /usr --libexecdir lib --sbindir bin --buildtype plain \
			-Denable-x11=true -Dxkb-config-root=/usr/share/X11/xkb -Dx-locale-root=/usr/share/X11/locale \
			-Denable-docs=false . build \
		&& meson compile -C build \
		&& DESTDIR="$instdir" meson install -C build)
	pkg_build $pkg
fi

# at-spi2-core
ver=2.38.0
download "https://gitlab.gnome.org/GNOME/at-spi2-core/-/archive/AT_SPI2_CORE_${ver//./_}/at-spi2-core-AT_SPI2_CORE_${ver//./_}.tar.bz2" "at-spi2-core-$ver.tar.bz2"
pkg="at-spi2-core-$ver-trusty-amd64"
if pkg_missing $pkg; then
	rm -rf at-spi2-core ; mkdir at-spi2-core
	tar -xjf "$downloaddir/at-spi2-core-$ver.tar.bz2" --strip-components=1 -C at-spi2-core
	(cd at-spi2-core \
		&& meson setup --prefix /usr --libexecdir lib --sbindir bin --buildtype plain \
			-D default_bus=dbus-broker -D docs=false . build \
		&& meson compile -C build \
		&& DESTDIR="$instdir" meson install -C build)
	pkg_build $pkg
fi

# libx264
download "https://code.videolan.org/videolan/x264/-/archive/stable/x264-stable.tar.bz2"
ver=$(date -r "$downloaddir/x264-stable.tar.bz2" +%Y%m%d)
pkg="libx264-$ver-trusty-amd64"
if pkg_missing $pkg; then
	tar -xjf "$downloaddir/x264-stable.tar.bz2" x264-stable
	(cd x264-stable \
		&& ./configure --prefix=/usr --enable-shared --enable-pic --enable-lto --disable-avs \
		&& make -j$(nproc) && make install DESTDIR="$instdir")
	pkg_build $pkg
fi

# libx265
ver=3.4
download "https://bitbucket.org/multicoreware/x265_git/get/${ver}.tar.bz2" "x265-${ver}.tar.bz2"
pkg="libx265-$ver-trusty-amd64"
if pkg_missing $pkg; then
	rm -rf x265-${ver} ; mkdir -p x265-${ver}/build{,-10,-12}
	tar -xjf "$downloaddir/x265-${ver}.tar.bz2" --strip-components=1 -C x265-${ver}
	(cd x265-${ver} \
		&& (cd build-12 && cmake -DCMAKE_INSTALL_PREFIX=/usr \
			-DHIGH_BIT_DEPTH=TRUE -DMAIN12=TRUE -DEXPORT_C_API=FALSE -DENABLE_CLI=FALSE -DENABLE_SHARED=FALSE \
			-Wno-dev ../source \
			&& make -j$(nproc)) \
		&& (cd build-10 && cmake -DCMAKE_INSTALL_PREFIX=/usr \
			-DHIGH_BIT_DEPTH=TRUE -DEXPORT_C_API=FALSE -DENABLE_CLI=FALSE -DENABLE_SHARED=FALSE \
			-Wno-dev ../source \
			&& make -j$(nproc)) \
		&& (cd build && cmake -DCMAKE_INSTALL_PREFIX=/usr \
			-DENABLE_SHARED=TRUE -DENABLE_HDR10_PLUS=TRUE -DEXTRA_LIB='x265_main10.a;x265_main12.a' \
			-DEXTRA_LINK_FLAGS='-L .' -DLINKED_10BIT=TRUE -DLINKED_12BIT=TRUE \
			-Wno-dev ../source) \
		&& ln -s ../build-10/libx265.a build/libx265_main10.a \
		&& ln -s ../build-12/libx265.a build/libx265_main12.a \
		&& (cd build && make -j$(nproc) && make install DESTDIR="$instdir"))
	pkg_build $pkg
fi

# libvpx
ver=1.9.0
download "https://chromium.googlesource.com/webm/libvpx/+archive/v${ver}.tar.gz" "libvpx-${ver}.tar.gz"
pkg="libvpx-$ver-trusty-amd64"
if pkg_missing $pkg; then
	sudo apt-get install -yqq yasm
	rm -rf libvpx ; mkdir libvpx
	tar -xzf "$downloaddir/libvpx-${ver}.tar.gz" -C libvpx
	(cd libvpx && mkdir bld && cd bld \
		&& ../configure --prefix=/usr --disable-examples --disable-tools --disable-docs --enable-shared --disable-static \
			--enable-vp8 --enable-vp9 --enable-runtime-cpu-detect --enable-libyuv \
			--disable-unit-tests \
		&& make -j$(nproc) && make install DESTDIR="$instdir")
	pkg_build $pkg
fi

# ffmpeg
ver=4.3
download "https://www.ffmpeg.org/releases/ffmpeg-${ver}.tar.xz"
pkg="ffmpeg-$ver-trusty-amd64"
if pkg_missing $pkg; then
	sudo apt-get install -yqq libass-dev libfreetype6-dev libsdl2-dev libtool libva-dev libvdpau-dev libvorbis-dev libxcb1-dev libxcb-shm0-dev libxcb-xfixes0-dev \
		zlib1g-dev pkgconf libglu1-mesa-dev libva-glx1 libglu1-mesa libbluray-dev libmodplug-dev libomxil-bellagio-dev libtheora-dev libvidstab-dev \
		libwebp-dev libxml2-dev libxv-dev libxvidcore-dev libgmp-dev libgnutls-dev libgsm1-dev libmp3lame-dev \
		libopencore-amrwb-dev libopencore-amrnb-dev libopenjpeg-dev libopus-dev libpulse-dev libsoxr-dev libspeex-dev
	tar -xJf "$downloaddir/ffmpeg-${ver}.tar.xz" ffmpeg-${ver}
	# AV1 codec: --enable-libaom --enable-libdav1d --enable-librav1e
	# Intel Media Dispatcher: --enable-libmfx
	# --enable-libsrt --enable-libssh --enable-libvmaf --enable-nvdec --enable-nvenc --enable-libopenjpeg
	(cd ffmpeg-${ver} && ./configure --prefix=/usr --enable-shared --disable-static --disable-doc --disable-debug --disable-stripping \
		--enable-fontconfig --enable-gmp --enable-gnutls --enable-gpl --enable-libass --enable-libbluray --enable-libdrm --enable-libfreetype \
		--enable-libfribidi --enable-libgsm --enable-libmodplug --enable-libmp3lame --enable-libopencore_amrnb \
		--enable-libopencore_amrwb --enable-libopus --enable-libpulse --enable-libsoxr --enable-libspeex \
		--enable-libtheora --enable-libvidstab --enable-libvorbis --enable-libvpx --enable-libwebp --enable-libx264 --enable-libx265 \
		--enable-libxcb --enable-libxml2 --enable-libxvid --enable-omx --enable-version3 \
		&& make -j$(nproc) && make install DESTDIR="$instdir")
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

# qt5
ver=5.12.10
pkg="qt5-bundle-$ver-trusty-amd64"
if pkg_missing $pkg; then
	sudo apt-get install -yqq libfontconfig1-dev libfreetype6-dev libx11-dev libxext-dev libxfixes-dev libxi-dev libxrender-dev libxcb1-dev libx11-xcb-dev libxcb-glx0-dev \
		fontconfig libasound2-dev libatk1.0-dev libavahi-client-dev libavahi-common-dev libcairo2-dev libdbus-1-dev libdbus-glib-1-dev libdrm-dev \
		libgbm-dev libgcrypt11-dev libgl1-mesa-dev libgl1-mesa-glx libgles2-mesa-dev libglu1-mesa-dev libgnutls-dev libgpg-error-dev \
		libgstreamer-plugins-base0.10-dev libgstreamer0.10-dev libharfbuzz-dev libice-dev libjpeg-dev libjpeg-turbo8-dev libjpeg8-dev libkrb5-dev libltdl-dev \
		libopenvg1-mesa-dev libp11-kit-dev libpango1.0-dev libpixman-1-dev libpq-dev libprotobuf-dev libpulse-dev libsm-dev libxcb-xinerama0-dev \
		libssl-dev libtasn1-6-dev libudev-dev libwayland-dev libxcb-dri2-0-dev libxcb-icccm4-dev libxcb-image0-dev libxcb-keysyms1-dev libxcb-present-dev libxcb-randr0-dev \
		libxcb-render-util0-dev libxcb-render0-dev libxcb-shape0-dev libxcb-shm0-dev libxcb-sync-dev libxcb-xfixes0-dev libxcb-xkb-dev libxcomposite-dev libxcursor-dev \
		libxdamage-dev libxft-dev libxinerama-dev libxrandr-dev libxshmfence-dev libxtst-dev libxxf86vm-dev mesa-common-dev \
		x11proto-composite-dev x11proto-damage-dev x11proto-dri2-dev x11proto-gl-dev x11proto-randr-dev x11proto-record-dev x11proto-xf86vidmode-dev x11proto-xinerama-dev \
		libegl1-mesa-drivers libglapi-mesa libicu-dev libxcb-res0-dev
	qt_modules=(qtbase qttools qtsvg qtx11extras qtscript qtxmlpatterns qtdeclarative qtimageformats qtquickcontrols qtquickcontrols2 qtwayland)
	build_qt() {
		local pkg="$1"
		local ver="$2"
		local verb=${ver%.*}
		local file="$pkg-everywhere-src-$ver.tar.xz"
		download "https://download.qt.io/archive/qt/$verb/$ver/submodules/$file"
		if [[ "$pkg" == "qtbase" ]]; then
			# NOTE: removed -system-xcb
			conf=(./configure -prefix /usr -opensource -confirm-license -nomake tests -nomake examples -dbus \
				-no-separate-debug-info -xcb -qpa xcb -no-eglfs -release -reduce-relocations \
				-optimized-qmake)
		else
			conf=(qmake)
		fi
		(mkdir -p $pkg && cd $pkg \
			&& echo -e "\033[93mExtracting $file...\033[m" \
			&& tar -xJpsf "$downloaddir/$file" --strip-components=1 \
			&& pkg_patch $pkg \
			&& "${conf[@]}" \
			&& make -j$(nproc) \
			&& INSTALL_ROOT="$instdir" make install)
	}
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

# plasma-wayland-protocols
ver=1.1.1
pkg="plasma-wayland-protocols-$ver-trusty-amd64"
download "https://download.kde.org/stable/plasma-wayland-protocols/${ver}/plasma-wayland-protocols-${ver}.tar.xz"
pkg="plasma-wayland-protocols-$ver-trusty-amd64"
if pkg_missing $pkg; then
	tar -xJf "$downloaddir/plasma-wayland-protocols-${ver}.tar.xz" "plasma-wayland-protocols-${ver}"
	(cd "plasma-wayland-protocols-${ver}" \
		&& cmake -B build -S . \
		&& cmake --build build \
		&& DESTDIR="$instdir" cmake --install build)
	pkg_build $pkg
fi

# kf5
ver=5.76.0
pkg="kf5-bundle-$ver-trusty-amd64"
if pkg_missing $pkg; then
	sudo apt-get install -yqq gperf flex libattr1-dev libcanberra-dev libxslt1-dev libboost1.55-all-dev python-setuptools
	
	build_kf5() {
		local pkg="$1"
		local ver="$2"
		local verb=${ver%.*}
		local file="$pkg-$ver.tar.xz"
		download "https://download.kde.org/stable/frameworks/$verb/$file"
		(mkdir -p $pkg/build && cd $pkg \
			&& echo -e "Extracting $file..." \
			&& tar -xJpsf "$downloaddir/$file" --strip-components=1 \
			&& pkg_patch $pkg \
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
		kwallet kio kactivities kparts
		#kdeclarative kirigami2 plasma-framework kcmutils knotifyconfig krunner kinit
		#kactivities-stats baloo kded kdewebkit kxmlrpcclient kdesignerplugin knewstuff ktexteditor
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

# plasma
ver=5.18.6
pkg="plasma-bundle-$ver-trusty-amd64"
if pkg_missing $pkg; then
	sudo apt-get install -yqq libfftw3-dev

	plasma_modules=(kdecoration breeze plasma-integration)
	build_plasma() {
		local pkg="$1"
		local ver="$2"
		local file="$pkg-$ver.tar.xz"
		download "https://download.kde.org/stable/plasma/$ver/$pkg-$ver.tar.xz"
		(mkdir -p $pkg/build && cd $pkg \
			&& echo -e "Extracting $file..." \
			&& tar -xJpsf "$downloaddir/$file" --strip-components=1 \
			&& pkg_patch $pkg \
			&& cd build \
			&& cmake \
				-DCMAKE_INSTALL_PREFIX=/usr \
				-DCMAKE_BUILD_TYPE=Release \
				-DBUILD_TESTING=OFF \
				.. \
			&& make -j$(nproc) \
			&& make install DESTDIR="$instdir")
	}
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
sudo apt-get install -yqq libicu-dev libopenal-dev zsync
# gstreamer module requirements
#sudo apt-get install -yqq libgstreamer1.0-dev libgstreamer-plugins-good1.0-dev libgstreamer-plugins-base1.0-dev
# phonon module requirements - it doesn't work that good will skip it
#sudo apt-get install -yqq libphonon4qt5-dev libphonon4qt5experimental-dev
# xine module requirements - requires libavformat, will skip building this plugin
# sudo apt-get install -yqq libxine2-dev

# appdir
#git clone --branch master --depth 1 https://github.com/maxrd2/subtitlecomposer.git "$builddir/subtitlecomposer"
git clone --branch master --depth 1 ssh://max@beeblebrox/home/max/projects/SubtitleComposer "$builddir/subtitlecomposer"
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
tar -xJf "$pkgdir/kf5-kio-5.76.0-trusty-amd64.tar.xz" -C "$appdir/"

# add kross
tar -xJf "$pkgdir/kross-5.52.0-trusty-amd64.tar.xz" -C "$appdir/"
tar -xJf "$pkgdir/kross-interpreters-18.08.3-trusty-amd64.tar.xz" -C "$appdir/"
[ -f "$appdir/usr/plugins/krossruby.so" ] && rm "$appdir/usr/plugins/krossruby.so"
rm "$appdir/usr/share/subtitlecomposer/scripts"/*.rb

# icons
tar -xJf "$pkgdir/kf5-breeze-icons-5.76.0-trusty-amd64.tar.xz" -C "$appdir/"

# style
tar -xJf "$pkgdir/plasma-breeze-5.18.6-trusty-amd64.tar.xz" -C "$appdir/"

# relocate x64 binaries from arch specific directory as required for appimages
[ -d "$appdir/usr/lib/x86_64-linux-gnu" ] && mv "$appdir/usr/lib/x86_64-linux-gnu"/* "$appdir/usr/lib" && rm -rf "$appdir/usr/lib/x86_64-linux-gnu"
[ -d "$appdir/usr/lib/libexec" ] && mv "$appdir/usr/lib/libexec" "$appdir/usr/libexec" && mv "$appdir/usr/libexec/kf5"/* "$appdir/usr/libexec" && rmdir "$appdir/usr/libexec/kf5"

# delete useless stuff
rm -rf "$appdir/usr/share/wallpapers"
rm -rf "$appdir/usr/share/icons/"{breeze-dark,Breeze_Snow,breeze_cursors,breeze/breeze-icons.rcc}
rm -rf "$appdir/usr/qml"
rm -rf "$appdir/usr/lib/cmake"
rm -rf "$appdir/usr/include"
rm -rf "$appdir/usr/mkspecs"
rm -f "$appdir/usr/bin"/[a-q]*
rm -f "$appdir/usr/share/metainfo/org.kde.breezedark.desktop.appdata.xml"

# appimage
cd "$appdir/.."
download "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage" linuxdeployqt
download "https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage" appimagetool
chmod +x "$downloaddir/linuxdeployqt" "$downloaddir/appimagetool"

execs=("$appdir/usr/lib/subtitlecomposer/"*.so)
#execs+=("$appdir/usr/plugins/styles/"*.so "$appdir/usr/plugins/iconengines/"*.so)
#plugins=("$appdir/usr/plugins/kf5/kio/"*.so) ; mkdir -p "$appdir/usr/plugins/kf5/kio/" ; cp -ax /usr/plugins/kf5/kio/*.so "$appdir/usr/plugins/kf5/kio/"
"$downloaddir/linuxdeployqt" "$appdir/usr/share/applications/org.kde.subtitlecomposer.desktop" -bundle-non-qt-libs "${execs[@]/#/-executable=}" #-extra-plugins="$(IFS=','; echo "${plugins[*]}")"

rm -v "$appdir/AppRun" && cp -v "$builddir/subtitlecomposer/pkg/misc/subtitlecomposer.sh" "$appdir/AppRun" && chmod +x "$appdir/AppRun"
rm -fv "$appdir/usr/lib/libxcb-dri2.so"* "$appdir/usr/lib/libxcb-dri3.so"*

"$downloaddir/appimagetool" "$appdir/" -u 'gh-releases-zsync|maxrd2|subtitlecomposer|continuous|Subtitle_Composer-*x86_64.AppImage.zsync'

#[ ! -f Subtitle_Composer*.AppImage.zsync ] && zsyncmake Subtitle_Composer*.AppImage
#mv Subtitle_Composer*.AppImage* "$pkgdir/"

# remove build dir
rm -rf "$builddir/subtitlecomposer"
