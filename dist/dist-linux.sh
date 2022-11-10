#!/bin/bash

set -e

if [[ "$#" -ne 2 ]]; then
    echo "Usage: $(basename "$0") <repository-dir> <arch>" >&2
    exit 1
fi

repo="$(realpath "$1")"
arch="$2"
version="$(sed -En 's/^VERSION = (.*)$/\1/p' "$repo/Makefile")"

docker run --rm -v "$repo:/chttpd" --platform "linux/$arch" alpine:latest sh -c "
set -e
apk add --no-cache gcc libc-dev make
cp -r /chttpd /tmp/chttpd
make -C /tmp/chttpd -B -j \"\$(nproc)\" LDFLAGS=\"-static -s\"
/tmp/chttpd/chttpd -v
tar -C /tmp/chttpd -czf \"/chttpd/chttpd-$version-linux-$arch.tar.gz\" chttpd
"
