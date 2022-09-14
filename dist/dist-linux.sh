#!/bin/bash

if [ "$#" -ne 2 ]; then
    echo "Usage: $(basename "$0") <repository-dir> <arch>" >&2
    exit 1
fi

repo="$(realpath "$1")"
arch="$2"
version="$(sed -n 's/^VERSION = \(.*\)$/\1/p' "$repo/Makefile")"

case "$arch" in
x86_64)
    platform=linux/amd64
    ;;
aarch64 | arm64)
    platform=linux/arm64
    ;;
*)
    echo "Error: unsupported architecture $arch" >&2
    exit 1
    ;;
esac

docker run --rm -v "$repo:/chttpd" --platform "$platform" alpine:latest sh -c "
apk add --no-cache build-base bash &&
cp -r /chttpd /tmp/chttpd &&
cd /tmp/chttpd &&
make -B -j \"\$(nproc)\" LDFLAGS=\"-static -s\" &&
tar czf \"/chttpd/chttpd-$version-linux-$arch.tar.gz\" chttpd
"
