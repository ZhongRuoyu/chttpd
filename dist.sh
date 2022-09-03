#!/bin/bash

if [ "$#" -eq 0 ]; then
    arch="$(uname -m)"
elif [ "$#" -eq 1 ]; then
    arch="$1"
else
    echo "Error: invalid number of arguments" >&2
    echo "Usage: $0 [arch]" >&2
fi

version="$(sed -n 's/^VERSION = \(.*\)$/\1/p' Makefile)"

case "$arch" in
x86_64 | amd64)
    image=amd64/alpine
    platform=linux/amd64
    arch=amd64
    ;;
aarch64 | arm64)
    image=arm64v8/alpine
    platform=linux/arm64
    arch=arm64
    ;;
*)
    echo "Error: unsupported architecture $arch" >&2
    exit 1
    ;;
esac

docker run --rm -v "$(pwd):/chttpd" --platform "$platform" "$image" sh -c "
apk add --no-cache build-base bash &&
cp -r /chttpd /tmp/chttpd &&
cd /tmp/chttpd &&
make clean &&
make -j \"\$(nproc)\" LDFLAGS='-static -s' &&
tar czf /chttpd/chttpd-$version-$arch.tar.gz chttpd
"
