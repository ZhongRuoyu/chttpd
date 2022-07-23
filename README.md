# chttpd

chttpd is a simple HTTP server written in C. It is able to serve static
contents that can be retrieved with HTTP `GET` requests. Concurrent
connections are supported.

chttpd complies with the C11 and POSIX.1-2008 standards.

## How to Build

To build chttpd, run `make`.

## How to Use

chttpd can be configured through command line options. For example, the
following command starts an HTTP server that serves contents under the `site`
directory under the current working directory, over `http://localhost:8080`.

```bash
/path/to/chttpd -h localhost -p 8080 -r site
```

chttpd logs the timestamp, IP, port, and request line of each request it
receives. Logs are printed to `stdout`, so if you want to save them, you may
want to use output redirection or `tee`.

For more information, run `/path/to/chttpd --help`.

## License

chttpd is licensed under [the MIT License](LICENSE).
