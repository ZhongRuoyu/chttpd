#!/bin/bash

docker run --rm -v "$(pwd):/chttpd" alpine sh -c "
apk add --no-cache build-base bash &&
cp -r /chttpd /tmp/chttpd &&
cd /tmp/chttpd &&
make clean &&
make -j \$(nproc) LDFLAGS='-static -s' &&
version=\$(./chttpd -v | sed 's/^chttpd \(.*\) (.*)$/\1/') &&
tar czf /chttpd/chttpd-\$version.tar.gz chttpd
"
