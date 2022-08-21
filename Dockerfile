FROM scratch
ADD chttpd-0.2.1.tar.gz /
ENTRYPOINT [ "/chttpd" ]
