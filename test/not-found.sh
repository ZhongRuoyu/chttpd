#!/bin/bash -e

port="${TEST_PORT:-8080}"

test_name=$(basename "$0" .sh)
t=out/test/$test_name
mkdir -p $t

nohup ./chttpd -h localhost -p $port -r $t >/dev/null 2>&1 &
chttpd_pid=$!

function cleanup {
    kill $chttpd_pid
    wait $chttpd_pid
    [ "$(jobs)" == "" ] || false
}
trap cleanup EXIT

sleep 1
curl -s http://localhost:$port/ | grep -q '^404 Not Found$'
