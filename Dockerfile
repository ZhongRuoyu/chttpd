# syntax=docker/dockerfile:1

FROM alpine AS build
WORKDIR /chttpd
COPY . /chttpd
RUN <<-"EOF"
    apk add --no-cache gcc libc-dev make
    make -B -j "$(nproc)" LDFLAGS="-static -s"
EOF

FROM scratch
COPY --from=build /chttpd/chttpd /
ENTRYPOINT [ "/chttpd" ]
