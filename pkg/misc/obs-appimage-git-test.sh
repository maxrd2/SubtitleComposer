#!/bin/sh

set -e # break on error

#trap '' 2  # Disable Ctrl+C

obs_project_dir="$(readlink -f "$PWD")"

[ ! -d "$obs_project_dir/.osc" -o ! -f "$obs_project_dir/subtitlecomposer.spec" ] \
		&& echo 'ERROR: OBS project dir is not current' 1>&2 \
		&& exit 1

git_project_dir="$(readlink -f "$(dirname "$(readlink -f "$0")")/../..")"
[ ! -d "$git_project_dir/.git" ] \
		&& echo 'ERROR: script is not in GIT project dir' 1>&2 \
		&& exit 1

osc_build_root="$(grep -E '^build-root = ' ~/.config/osc/oscrc | sed -r \
	-e 's/^build-root =\s+(.*)\s*$/\1/' \
	-e 's|%\(repo\)s|AppImage|g' \
	-e 's|%\(arch\)s|x86_64|g')"

appimage_dest_dir="$(readlink -f "$(grep -E '^# appimage-dest-dir = ' ~/.config/osc/oscrc | sed -r \
	-e 's/^# appimage-dest-dir =\s+(.*)\s*$/\1/' \
	-e 's|%\(repo\)s|AppImage|g' \
	-e 's|%\(arch\)s|x86_64|g')")"

branch='HEAD'
ver='test-git'
gitrev="$(git -C "$git_project_dir" rev-parse --short=12 $branch)"
gitver="$(git -C "$git_project_dir" describe --always --tags --long --abbrev=8 $branch)"
ver="$(echo $gitver | sed -E -e 's/^v(.+)-([0-9]+)-g(.+)$/\1-1+'$ver'\2-\3/')"

echo -e "\e[1;32m### Preparing to build\e[1;37m - version $ver\e[m"

cp -vf "$git_project_dir/pkg/misc/appimage.yml" "appimage.yml"

mkdir -p "$obs_project_dir/subtitlecomposer-test-git"
rsync -av "$git_project_dir"/{pkg,src} subtitlecomposer-test-git/.

old_ver="$(grep -E '^\s+- app_version=' appimage.yml)"

test_cleanup() {
	rm -fv "subtitlecomposer-${ver}.tar.xz"
	sed -r \
		-e "/^\s+- app_version=/ c \\$old_ver" \
		-e '/^  - make -j\$\(nproc\)$/ c \\  - make' \
		-i appimage.yml
}
trap test_cleanup EXIT

# update build version
sed -r \
	-e 's/^(\s+- app_version=)"[^"]+".*$/\1"'"$ver"'"/g' \
	-e 's/^(\s+- make)$/\1 -j$(nproc)/g' \
	-i appimage.yml

# create build archive
tar -cJf "subtitlecomposer-${ver}.tar.xz" subtitlecomposer-test-git

osc build AppImage x86_64 appimage.yml

echo -e "\e[1;32m### Build successful\e[1;37m - root '$osc_build_root'\e[m"

app_image="$(ls "$osc_build_root/.mount/.build.packages/OTHER"/SubtitleComposer-*-x86_64.AppImage)"
if [ ! -z "$appimage_dest_dir" ]; then
	cp -va "$app_image" "$appimage_dest_dir/SubtitleComposer-x86_64.AppImage"
	chmod +x "$appimage_dest_dir/SubtitleComposer-x86_64.AppImage"
	echo -e "\e[1;32m###\e[1;37m $appimage_dest_dir/SubtitleComposer-x86_64.AppImage\e[m"
fi
