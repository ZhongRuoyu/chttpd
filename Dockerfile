FROM scratch
ADD chttpd-0.2.0.tar.gz /
ENTRYPOINT [ "/chttpd" ]
