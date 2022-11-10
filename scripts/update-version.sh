#!/bin/sh

set -e

get_version() {
    local repo="$1"
    if [ -f "$repo/Makefile" ]; then
        if version="$(sed -En 's/^VERSION = (.*)$/\1/p' "$repo/Makefile")"; then
            local version="$(sed -En 's/^VERSION = (.*)$/\1/p' "$repo/Makefile")"
            echo "$version"
        fi
    fi
}

get_git_hash() {
    local repo="$1"
    if [ -f "$repo/.git/HEAD" ]; then
        if grep -Eq '^ref: ' "$repo/.git/HEAD"; then
            local head="$(sed -En 's/^ref: (.*)$/\1/p' "$repo/.git/HEAD")"
            if [ -f "$repo/.git/$head" ]; then
                cat "$repo/.git/$head"
            else
                local head_ref="$(sed -En "s:^(.*) $head\$:\\1:p" "$repo/.git/packed-refs")"
                echo "$head_ref"
            fi
        else
            cat "$repo/.git/HEAD"
        fi
    fi
}

read_prev_version() {
    local file="$1"
    if [ -f "$file" ]; then
        if grep -Eq '^// Version: ' "$file"; then
            local prev_version="$(sed -En 's|^// Version: (.*)$|\1|p' "$file")"
            echo "$prev_version"
        fi
    fi
}

read_prev_git_hash() {
    local file="$1"
    if [ -f "$file" ]; then
        if grep -Eq '^// Git hash: ' "$file"; then
            local prev_git_hash="$(sed -En 's|^// Git hash: (.*)$|\1|p' "$file")"
            echo "$prev_git_hash"
        fi
    fi
}

if [ "$#" -ne 2 ]; then
    echo "Usage: $(basename "$0") <repository-dir> <output-file>" >&2
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

if [ -z "$prev_version" ] ||
    [ -z "$prev_git_hash" ] ||
    [ "$current_version" != "$prev_version" ] ||
    [ "$current_git_hash" != "$prev_git_hash" ]; then
    cat <<EOF | tee "$output_file" >/dev/null
// Generated by $(basename "$0"). DO NOT EDIT.
// Version: $current_version
// Git hash: $current_git_hash

const char *GetVersion(void) {
    return "$version_string";
}
EOF
fi
