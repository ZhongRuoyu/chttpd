#!/bin/bash -e

get_version() {
    repo="$1"
    if [ -f "$repo/Makefile" ]; then
        if version="$(grep '^VERSION = ' "$repo/Makefile" | sed 's/.* = //')"; then
            echo "$version"
        fi
    fi
}

get_git_hash() {
    repo="$1"
    if [ -f "$repo/.git/HEAD" ]; then
        if grep -q '^ref: ' "$repo/.git/HEAD"; then
            cat "$repo/.git/$(sed 's/^ref: //' "$repo/.git/HEAD")"
        else
            cat "$repo/.git/HEAD"
        fi
    fi
}

read_prev_version() {
    path="$1"
    if [ -f "$path" ]; then
        if prev_version="$(sed -n 's/^\/\/ Version: \(.*\)$/\1/p' "$path")"; then
            echo "$prev_version"
        fi
    fi
}

read_prev_git_hash() {
    path="$1"
    if [ -f "$path" ]; then
        if prev_git_hash="$(sed -n 's/^\/\/ Git hash: \(.*\)$/\1/p' "$path")"; then
            echo "$prev_git_hash"
        fi
    fi
}

if [ "$#" -ne 2 ]; then
    echo "Usage: $(basename $(readlink -f "$0")) <repository-dir> <output-file>" >&2
    exit 1
fi

repo="$1"
output_file="$2"

current_version="$(get_version "$repo")"
current_git_hash="$(get_git_hash "$repo")"
prev_version="$(read_prev_version "$output_file")"
prev_git_hash="$(read_prev_git_hash "$output_file")"

if [ -z "$current_version" ]; then
    echo "Error: cannot get current version" >&2
    exit 1
fi

if [ -n "$current_git_hash" ]; then
    version_string="chttpd $current_version ($current_git_hash)"
else
    version_string="chttpd $current_version"
fi

if [ -z "$prev_version" \
    -o -z "$prev_git_hash" \
    -o "$current_version" != "$prev_version" \
    -o "$current_git_hash" != "$prev_git_hash" ]; then
    cat <<EOF >"$output_file"
// Generated by $(basename $(readlink -f "$0")).
// Version: $current_version
// Git hash: $current_git_hash

const char *GetVersion(void) {
    return "$version_string";
}
EOF
fi
