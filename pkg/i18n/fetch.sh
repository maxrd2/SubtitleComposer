#!/bin/sh

set -e

i18npath="$(readlink -f "$(dirname "$0")")"
gitpath="$(readlink -f "$i18npath/../..")"

[ -d "$gitpath/po" ] || [ -d "$gitpath/poqm" ] && (
	echo -e "ERROR: po/poqm directories already exist\n\nrm -rf \"$gitpath\"/po{,qm}"
	exit 1
)

releaseme_path="$i18npath/releaseme"
fetchpo="$releaseme_path/fetchpo.rb"

[ ! -d "$releaseme_path" ] && git clone git@invent.kde.org:sdk/releaseme.git "$releaseme_path"

$fetchpo --origin trunk --project multimedia/subtitlecomposer --output-dir po --output-poqm-dir poqm .

patch -N -p1 -F5 -d "$gitpath" -r /dev/null --no-backup-if-mismatch <<"EOF" || true
diff --git a/CMakeLists.txt b/CMakeLists.txt
index 062c069..b6ce7e6 100644
--- a/CMakeLists.txt
+++ b/CMakeLists.txt
@@ -28,7 +28,10 @@ find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
        Auth Config ConfigWidgets CoreAddons I18n KIO XmlGui
        Sonnet Codecs TextWidgets WidgetsAddons)

-#PO_SUBDIR
+
+find_package(KF5I18n CONFIG REQUIRED)
+ki18n_install(po)
+
 add_subdirectory(src)

 add_custom_target(nsis COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/pkg/mingw/nsi-installer.sh" `${CMAKE_C_COMPILER} -dumpmachine`)
EOF

git="git -C $gitpath"
files=(po CMakeLists.txt)
if [ "$($git describe HEAD --all)" = "heads/obs/latest" ]; then
	gitver="$($git describe --always --tags --abbrev=10 kde/master)"
	echo "Updating version to $gitver"
	sed -E "s|SUBTITLECOMPOSER_VERSION_STRING|\"${gitver#v}\"|" -i $gitpath/src/main.cpp
	files+=(src/main.cpp)
	$git add "${files[@]}"
	$git commit -m "Added i18n - $gitver"
else
	echo -e "\n$git add ${files[@]} && $git commit -c 'Added i18n - $gitver'"
fi
