#!/bin/bash

set -e

if [[ "$#" -ne 1 ]]; then
    echo "Usage: $(basename "$0") <repository-dir>" >&2
    exit 1
fi

repo="$1"
version="$(sed -En 's/^VERSION = (.*)$/\1/p' "$repo/Makefile")"

make -C "$repo" -B -j "$(sysctl -n hw.logicalcpu)" UNIVERSAL_BINARY=1
tar -C "$repo" -czf "chttpd-$version-macos-universal.tar.gz" chttpd
