#!/bin/bash

if [ "$#" -ne 2 ]; then
    echo "Usage: $(basename "$0") <repository-dir> <arch>" >&2
    exit 1
fi

repo="$(realpath "$1")"
arch="$2"
version="$(sed -n 's/^VERSION = \(.*\)$/\1/p' "$repo/Makefile")"

docker run --rm -v "$repo:/chttpd" --platform "linux/$arch" alpine:latest sh -c "
apk add --no-cache gcc libc-dev make &&
cp -r /chttpd /tmp/chttpd &&
cd /tmp/chttpd &&
make -B -j \"\$(nproc)\" LDFLAGS=\"-static -s\" &&
tar czf \"/chttpd/chttpd-$version-linux-$arch.tar.gz\" chttpd
"
