FROM scratch
ARG TARGETARCH
ADD chttpd-0.2.1-$TARGETARCH.tar.gz /
ENTRYPOINT [ "/chttpd" ]
