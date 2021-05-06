#!/bin/bash

set -e # break on error

#trap '' 2  # Disable Ctrl+C

gpg_key="EF9D9B26"

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
	-e 's|%\(repo\)s|Windows|g' \
	-e 's|%\(arch\)s|x86|g')"

dist_tar="$(ls "$obs_project_dir"/subtitlecomposer-*.tar.xz)"

[[ ! -f "$dist_tar" ]] && echo "Archive '$dist_tar' doesn't exist.." && exit 1

cleanup() {
	[ ! -z "$pid_tail" ] && echo "Terminating logger" && kill "$pid_tail"
	[ ! -z "$docker_id" ] && echo "Terminating container $docker_id" && docker container kill "$docker_id"
	return 0
}

build() {
	pid_tail=
	docker_id=
	trap cleanup EXIT

	echo "Extracting sources into '$osc_build_root'..."
	rm -rf "$osc_build_root"
	install -m 0700 -o "$user_id" -d "$osc_build_root" "$osc_build_root/dist-win32"
	sudo -u "$user_name" tar -xf "$dist_tar" --strip-components=1 -C "$osc_build_root"

	echo "Logging to '$log_file'..."
	echo "Building archive '$dist_tar'..." > "$log_file"
	chown "$user_id" "$log_file"
	tail -f "$log_file" &
	pid_tail=$!

	echo "Starting docker build..."
	docker_id="$(docker container run -d --rm -v "$osc_build_root":/home/devel -it maxrd2/arch-mingw /bin/bash -c 'bash pkg/mingw/build.sh >>dist-win32/build.log 2>&1')"
	docker container wait $docker_id
	docker_id=
	return 0
}

user_id="$UID"
user_name="$USER"
log_file="$osc_build_root/dist-win32/build.log"
sudo bash -c "set -e; $(declare -f build cleanup); $(declare -p gpg_key obs_project_dir git_project_dir osc_build_root dist_tar user_id user_name log_file); build"
echo "Build completed successfully."

gpg_sign() {
	gpg --yes --output "$1.sig" --default-key "$gpg_key" --armor --detach-sign "$1"
	gpg --verify "$1.sig"
}

echo "Preparing distribution..."
mv "$osc_build_root/build/SubtitleComposerSetup.exe" "$osc_build_root/dist-win32/"
gpg_sign "$osc_build_root/dist-win32/SubtitleComposerSetup.exe"
gpg_sign "$log_file"

echo "Uploading distribution..."
rsync -zav --delete "$osc_build_root/dist-win32/." "subtitlecomposer.dist:web/subtitlecomposer.smoothware.net/public_html/win32/."
