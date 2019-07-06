#!/bin/bash

gh_repo=maxrd2/SubtitleComposer

set -e

[[ ! -f "$HOME/.github/token" ]] && echo "\e[1;31mERROR:\e[m Cannot find GitHub token." 1>&2 && exit 1
. "$HOME/.github/token"

cd "$(dirname "$0")/../.."

#curl=(curl -u "maxrd2:${GITHUB_TOKEN}")
curl=(curl -H "Authorization: token ${GITHUB_TOKEN}")
api_url="https://api.github.com/repos/$gh_repo"

release_from_repo() {
	gh_is_tag=
	gh_release="$(cd "$(dirname "$0")" && git describe --exact-match 2>/dev/null)" && gh_is_tag=1
	[[ -z "$gh_release" ]] && gh_release="$(cd "$(dirname "$0")" && git branch --show-current)"
	[[ "$gh_release" = "master" ]] && gh_release="continuous"
	[[ -z "$gh_release" ]] && echo "\e[1;31mERROR:\e[m unable to figure current branch/tag." 1>&2 && exit 1
	export gh_is_tag gh_release
}

release_update() {
	local url="$api_url/releases/tags/$gh_release"
	echo -e "Getting release info from '\e[1;39m$url\e[m'..."
	"${curl[@]}" -s -XGET "${url}" | jq . >.github_release
	local id="$(jq -r .id .github_release)"
	if [[ -z "$id" || "$id" = "null" ]]; then
		# create release
		"${curl[@]}" -s "$api_url/releases" -XPOST --data '{"tag_name":"'"$gh_release"'", "name":"'"$gh_release_name"'", "body":"'"$gh_release_body"'", "draft":false, "prerelease":'$gh_prerelease'}' >.github_release
	else
		# update release
		"${curl[@]}" -s "$api_url/releases/$id" -XPOST --data '{"tag_name":"'"$gh_release"'", "name":"'"$gh_release_name"'", "body":"'"$gh_release_body"'", "draft":false, "prerelease":'$gh_prerelease'}' >.github_release
	fi
}

asset_exists() {
	declare -i i=0
	local pathname="$1"
	local basename="$(basename "$pathname")"
	local filesize="$(stat --printf="%s" "$pathname")"
	local assets=(`jq -r '.assets[].name' .github_release`)
	for asset_name in "${assets[@]}"; do
		if [[ "$asset_name" = "$basename" ]]; then
			if [[ "$filesize" -ne "$(jq -r '.assets['$i'].size' .github_release)" ]]; then
				echo -e "Delete '\e[1;31m$asset_name\e[m' - filesize mismatch..."
				"${curl[@]}" -s -XDELETE "$(jq -r '.assets['$i'].url' .github_release)"
				return 1
			fi
			return 0
		fi
		i=i+1
	done
	return 1
}

asset_delete() {
	declare -i i=0
	local pathname="$1"
	local basename="$(basename "$pathname")"
	local filesize="$(stat --printf="%s" "$pathname")"
	local assets=(`jq -r '.assets[].name' .github_release`)
	for asset_name in "${assets[@]}"; do
		if [[ "$asset_name" = "$basename" ]]; then
			echo -e "Delete '\e[1;31m$asset_name\e[m'..."
			"${curl[@]}" -s -XDELETE "$(jq -r '.assets['$i'].url' .github_release)"
			return 0
		fi
		i=i+1
	done
	return 1
}

asset_upload() {
	local upload_url="$(jq -r '.upload_url|gsub("{.*}";"")' .github_release)?name="

	# upload assets
	for pathname in "$@"; do
		local basename="$(basename "$pathname")"

		asset_exists "$pathname" && asset_delete "$pathname"

		local url="$upload_url$(echo "$basename" | sed -e 's!%!%25!g;s! !%20!g;s!:!%3A!g;s!+!%2B!g')"
		echo -e "Uploading '\e[1;33m$basename\e[m' to '$url'..."
		asset="$("${curl[@]}" \
			-H "Accept: application/vnd.github.manifold-preview" \
			-H "Content-Type: application/octet-stream" \
			--data-binary "@$pathname" \
			"$url")"
		[[ "$(echo "$asset" | jq -r .errors)" != "null" ]] && echo "$asset" | jq -C . && return 1
		local remotename="$(echo "$asset" | jq -r .name)"
		if [[ "$basename" != "$remotename" ]]; then
			echo "\e[1;31mWARNING:\e[m GitHub renamed our file '$basename' to '$(echo "$asset" | jq -r .name)'..."
			echo "$asset" | jq -C .
		fi
	done
}

# Build
build=pkg/mingw/docker-build.sh
echo -e "Starting docker build '\e[1;39m$build\e[m'..."
$build | tee mingw-build-log.txt

# Create/Update release
gh_release="win32" # Don't use release_from_repo just yet
gh_prerelease="true"
gh_release_name="Experminetal Win32 Build"
gh_release_body="Experimental windows build from master."
release_update "$gh_release"
# jq -C . .github_release

# Upload files
asset_upload "$PWD/$build" "$PWD/mingw-build-log.txt" "$PWD/build/SubtitleComposerSetup.exe"
