#!/bin/bash

#gh_repo="$TRAVIS_REPO_SLUG"
gh_repo=maxrd2/SubtitleComposer

set -e

[[ ! -z "$TRAVIS_EVENT_TYPE" && "$TRAVIS_EVENT_TYPE" != "push" ]] \
	&& echo -e "\e[1;31mWARNING:\e[m Not releasing '$TRAVIS_EVENT_TYPE' event." 1>&2 \
	&& exit

[[ -z "$1" || ! -f "$2" ]] && echo -e "\e[1;33mUsage:\e[1;39m github-release.sh <platform> <file> ...\e[m" 1>&2 && exit 1

if [[ -z "$GITHUB_TOKEN" ]]; then
	[[ ! -f "$HOME/.github/token" ]] && echo "\e[1;31mERROR:\e[m Cannot find GitHub token." 1>&2 && exit 1
	. "$HOME/.github/token"
fi

cd "$(dirname "$0")/../.."

#curl=(curl -u "maxrd2:${GITHUB_TOKEN}")
curl=(curl -H "Authorization: token ${GITHUB_TOKEN}")
api_url="https://api.github.com/repos/$gh_repo"

release_from_repo() {
	gh_is_tag=

	if [[ -z "$TRAVIS_BRANCH" ]]; then
		gh_branch="$(cd "$(dirname "$0")" && git describe --exact-match 2>/dev/null)" && gh_is_tag=1
		[[ -z "$gh_branch" ]] && gh_branch="$(cd "$(dirname "$0")" && git branch --show-current)"
	else
		gh_branch="$TRAVIS_BRANCH"
		[[ "$TRAVIS_BRANCH" == "$TRAVIS_TAG" ]] && gh_is_tag=1
	fi
	[[ -z "$gh_branch" ]] && echo "\e[1;31mERROR:\e[m unable to figure current branch/tag." 1>&2 && exit 1
	export gh_is_tag gh_branch
}

release_update() {
	local ght_appimage="Instructions on using AppImage can be found [here](https://github.com/maxrd2/subtitlecomposer/wiki/AppImage-HOWTO)"
	if [[ $gh_is_tag -eq 1 ]]; then
		local gh_tag="$gh_branch"
		local gh_rel_name="Release $gh_tag"
		local gh_rel_body="$ght_appimage\n\nThis is ***stable release $gh_tag build***."
		local gh_prerelease='false'
		echo -e "Processing '\e[1;39m$gh_tag\e[m' release from tag '\e[1;39m$gh_tag\e[m'..."
	else
		if [[ "$gh_branch" = "master" ]]; then
#			local gh_tag="testing-refs"
#			local gh_rel_name="Dummy entry"
			local gh_tag="continuous"
			local gh_rel_name="Latest Development Build - git master"
			local gh_rel_body="$ght_appimage\n\nThis is the ***latest development build***, below you can find stable release builds."
		else
			local gh_tag="testing-$gh_branch"
			local gh_rel_name="Experimental Build - $gh_branch branch"
			local gh_rel_body="$ght_appimage\n\nThis is ***$gh_branch experimental build*** for testing new features."
		fi
		local gh_prerelease='true'
		echo -e "Processing '\e[1;39m$gh_tag\e[m' release from branch '\e[1;39m$gh_branch\e[m'..."

		# create/point release tag to branch head
		sha="$("${curl[@]}" -s -XGET "$api_url/git/refs/heads/$gh_branch" | jq -r .object.sha)"
		http=$("${curl[@]}" -s -XPATCH "$api_url/git/refs/tags/$gh_tag" --data '{"sha":"'"$sha"'"}' --output /dev/null --write-out '%{http_code}')
		[[ $http -lt 200 || $http -ge 300 ]] && http=$("${curl[@]}" -s -XPOST "$api_url/git/refs" --data '{"ref":"refs/tags/'"$gh_tag"'", "sha":"'"$sha"'"}' --output /dev/null --write-out '%{http_code}')
		[[ $http -lt 200 || $http -ge 300 ]] && echo -e "\e[1;31mERROR:\e[m unable to update/create tag '\e[1;39m$gh_tag\e[m'." && exit 1
	fi

	local ght_travis="Travis CI $gh_platform build log"
	local url="$api_url/releases/tags/$gh_tag"
	echo -e "Getting release info from '\e[1;39m$url\e[m'..."
	"${curl[@]}" -s -XGET "${url}" | jq . >.github_release
	local id="$(jq -r .id .github_release)"
	if [[ -z "$id" || "$id" = "null" ]]; then
		# create new release
		[[ ! -z "$TRAVIS_JOB_ID" ]] && gh_rel_body="$gh_rel_body\n\n$ght_travis: https://travis-ci.org/$TRAVIS_REPO_SLUG/jobs/$TRAVIS_JOB_ID"
		echo -e "Creating release '\e[1;39m$gh_tag\e[m'..."
		"${curl[@]}" -s -XPOST "$api_url/releases" --data '{"tag_name":"'"$gh_tag"'", "name":"'"$gh_rel_name"'", "body":"'"$gh_rel_body"'", "draft":false, "prerelease":'$gh_prerelease'}' >.github_release
	else
		# update existing release with new build log
		gh_rel_body="$(jq -r .body .github_release | grep -v "$ght_travis" | sed -e 's|\r||g' -e ':a;N;$!ba;s/\n/\\n/g;s/\r//g')"
		[[ ! -z "$TRAVIS_JOB_ID" ]] && gh_rel_body="$gh_rel_body\n$ght_travis: https://travis-ci.org/$TRAVIS_REPO_SLUG/jobs/$TRAVIS_JOB_ID"
		echo -e "Updating release '\e[1;39m$gh_tag\e[m'..."
		"${curl[@]}" -s -XPOST "$api_url/releases/$id" --data '{"tag_name":"'"$gh_tag"'", "name":"'"$gh_rel_name"'", "body":"'"$gh_rel_body"'", "draft":false, "prerelease":'$gh_prerelease'}' >.github_release
	fi
	id="$(jq -r .id .github_release)"
	[[ -z "$id" || "$id" = "null" ]] && echo -e "\e[1;31mERROR:\e[m unable to update/create release '\e[1;39m$gh_tag\e[m'." && exit 1 || true
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

gh_platform="$1"

# Create/Update release
release_from_repo
release_update
#jq -C . .github_release

# Upload files
asset_upload "${@:2}"
