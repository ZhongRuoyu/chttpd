#!/bin/bash -e

port="${TEST_PORT:-8080}"

test_name=$(basename "$0" .sh)
t=out/test/$test_name
mkdir -p $t

cat >$t/index.html <<EOF
OK
EOF

cat >$t/request.py <<EOF
#!/usr/bin/env python3
import socket
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(("localhost", $port))
s.send(b"GET /../ HTTP/1.1\r\nHost: localhost:$port\r\n\r\n")
response = s.recv(4096).decode("utf-8")
body = response[response.index("\r\n\r\n") + 4:]
print(body, end="")
s.close()
EOF

exit 0

nohup ./chttpd -h localhost -p $port -r $t >/dev/null 2>&1 &
chttpd_pid=$!

function cleanup {
    kill $chttpd_pid
    wait $chttpd_pid
    [ "$(jobs)" == "" ] || false
}
trap cleanup EXIT

sleep 1
/usr/bin/env python3 $t/request.py 2>/dev/null | grep -q '^400 Bad Request$'
