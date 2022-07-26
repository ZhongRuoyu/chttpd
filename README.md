# chttpd

chttpd is a simple, safe, and efficient HTTP server which can serve static
contents retrievable with HTTP `GET` requests. Incoming requests can be
handled concurrently.

Written in C, chttpd conforms to the C11 and POSIX.1-2008 standards.

## How to Build

To build chttpd, run the following. A binary named `chttpd` will be built.

```bash
git clone https://github.com/ZhongRuoyu/chttpd.git
cd chttpd
git checkout v0.1.0
make -j$(nproc)
```

Your C compiler and C standard library need to have support for C11 and
POSIX.1-2008.

## How to Use

chttpd is configured through command line options. For example, the following
command starts an HTTP server that serves contents under the `site` directory,
over `http://<hostname>:8080`.

```bash
chttpd -p 8080 -r site
```

chttpd can also be run as a daemon (hence its name), when `-d` is passed. For
example, the following command runs chttpd as a daemon which does the same
thing as above, but also saves the log output to `chttpd.log`.

```bash
chttpd -d -p 8080 -r site -l chttpd.log
```

For more information, run `chttpd --help`.

## License

chttpd is licensed under [the MIT License](LICENSE).
