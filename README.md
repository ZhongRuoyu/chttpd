# chttpd

chttpd is a simple HTTP server written in C which conforms to the C11 and
POSIX.1-2008 standards. It is able to serve static contents retrievable with
HTTP `GET` requests. Incoming requests can be handled concurrently.

## How to Build

To build chttpd, run the following. A binary named `chttpd` will be built.

```bash
git clone https://github.com/ZhongRuoyu/chttpd.git
cd chttpd
git checkout v0.1.0
make -j$(nproc)
```

Your C compiler and C standard library need to have support
for C11 and POSIX.1-2008.

## How to Use

chttpd can be configured through command line options. For example, the
following command starts an HTTP server that serves contents under the `site`
directory under the current working directory, over `http://<hostname>:8080`.

```bash
chttpd -p 8080 -r site
```

chttpd logs the timestamp, IP, port, and request line of each request it
receives. Logs are printed to `stdout`, so if you want to save them, you may
want to use output redirection or `tee`.

For more information, run `chttpd --help`.

## License

chttpd is licensed under [the MIT License](LICENSE).
