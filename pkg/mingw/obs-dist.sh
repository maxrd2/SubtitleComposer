#!/bin/bash

set -e # break on error

gpg_key="EF9D9B26"
obs_dir="$HOME/projects/subtitlecomposer-obs/home:maxrd2/subtitlecomposer"
build_dir="$HOME/tmp/dist-subtitlecomposer"

dist_tar="$(ls "$obs_dir"/subtitlecomposer-*.tar.xz)"

gpg_sign() {
	gpg --yes --output "$1.sig" --default-key "$gpg_key" --armor --detach-sign "$1"
	gpg --verify "$1.sig"
}

[[ ! -f "$dist_tar" ]] && echo "Archive '$dist_tar' doesn't exist.." && exit 1

echo "Extracting sources into '$build_dir'..."
rm -rf "$build_dir"
mkdir -p "$build_dir/dist-win32"
tar -xf "$dist_tar" --strip-components=1 -C "$build_dir"

echo "Starting docker build..."
echo "Building archive '$dist_tar'..." > "$build_dir/dist-win32/build.log"
sudo docker run --rm -v "$build_dir":/home/devel -it maxrd2/arch-mingw /bin/bash -c 'bash pkg/mingw/build.sh' >> "$build_dir/dist-win32/build.log"

echo "Build completed..."
ls -la "$build_dir/build"

echo "Preparing distribution..."
mv "$build_dir/build/SubtitleComposerSetup.exe" "$build_dir/dist-win32/"
gpg_sign "$build_dir/dist-win32/SubtitleComposerSetup.exe"
gpg_sign "$build_dir/dist-win32/build.log"

echo "Uploading distribution..."
rsync -zav --delete "$build_dir/dist-win32/." "subtitlecomposer.dist:web/subtitlecomposer.smoothware.net/public_html/win32/."

echo "Removing '$build_dir'..."
rm -rf "$build_dir"
