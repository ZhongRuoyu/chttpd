#!/bin/bash -e

port="${TEST_PORT:-8080}"

test_name=$(basename "$0" .sh)
t=out/test/$test_name
mkdir -p $t

cat >$t/index.html <<EOF
OK
EOF

nohup ./chttpd -h localhost -p $port -r $t >/dev/null 2>&1 &
chttpd_pid=$!

function cleanup {
    kill $chttpd_pid
    wait $chttpd_pid
    [ "$(jobs)" == "" ] || false
}
trap cleanup EXIT

sleep 1
! curl -s -H "Host: example.com" http://localhost:$port/
curl -s -H "Host: localhost" http://localhost:$port/ | grep -q '^OK$'
