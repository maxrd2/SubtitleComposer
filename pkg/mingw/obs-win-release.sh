#!/bin/bash

set -e # break on error

#trap '' 2  # Disable Ctrl+C

gpg_key="EF9D9B26"

obs_project_dir="$(readlink -f "$PWD")"

[ ! -d "$obs_project_dir/.osc" -o ! -f "$obs_project_dir"/subtitlecomposer*.spec ] \
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

osc service run

dist_cpio="$(ls "$obs_project_dir"/_service:obs_scm:subtitlecomposer-*.obscpio)"

docker_id=obs_win32_build

[[ ! -f "$dist_cpio" ]] && echo "Archive '$dist_cpio' doesn't exist.." && exit 1

cleanup() {
	[ ! -z "$pid_tail" ] && echo "Terminating logger" && kill "$pid_tail"
	[ ! -z "$(docker container ls -q -f "name=$docker_id")" ] && echo "Terminating container $docker_id" && docker container kill "$docker_id"
	return 0
}

build() {
	pid_tail=
	trap cleanup EXIT

	echo "Extracting sources into '$osc_build_root'..."
	rm -rf "$osc_build_root"
	install -m 0700 -o "$user_id" -d "$osc_build_root" "$osc_build_root/dist-win32"
	sudo -u "$user_name" cpio -i -d -F "$dist_cpio" -D "$osc_build_root"
	sudo -u "$user_name" cp -vax "$git_project_dir/pkg/mingw/." "$osc_build_root"/subtitlecomposer-*/pkg/mingw/.

	echo "Logging to '$log_file'..."
	echo "Building archive '$dist_cpio'..." > "$log_file"
	chown "$user_id" "$log_file"

	[ -z "$(docker container ls -aq -f "name=$docker_id")" ] && {
		echo "Creating docker container..."
		docker container create -v "$osc_build_root":/home/devel --name "$docker_id" -it maxrd2/arch-mingw /bin/bash -c 'bash subtitlecomposer-*/pkg/mingw/build.sh >>dist-win32/build.log 2>&1'
	}
	echo "Starting docker build..."
	docker container start "$docker_id"

	tail -f "$log_file" &
	pid_tail=$!

	docker container wait "$docker_id"
	return 0
}

user_id="$UID"
user_name="$USER"
log_file="$osc_build_root/dist-win32/build.log"
sudo bash -c "set -e; $(declare -f build cleanup); $(declare -p gpg_key obs_project_dir git_project_dir osc_build_root dist_cpio user_id user_name docker_id log_file); build"
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
