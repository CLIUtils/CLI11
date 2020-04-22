FROM ubuntu:18.04

RUN apt-get update \
 && apt-get install -y --no-install-recommends \
        g++=4:7.4.0-1ubuntu2.3 \
        wget=1.19.4-1ubuntu2.2 \
        libidn11=1.33-2.1ubuntu1.2 \
        ca-certificates=20180409 \
        make=4.1-9.1ubuntu1 \
        git=1:2.17.1-1ubuntu0.7 \
 && apt-get clean \
 && rm -rf /var/lib/apt/lists/*

COPY entrypoint.sh /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]
