#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <repository-dir>" >&2
    exit 1
fi

repo="$1"
version="$(sed -n 's/^VERSION = \(.*\)$/\1/p' "$repo/Makefile")"

make -C "$repo" -B -j "$(sysctl -n hw.logicalcpu)" UNIVERSAL_BINARY=1
tar -czf "chttpd-$version-macos-universal.tar.gz" "$repo/chttpd"
