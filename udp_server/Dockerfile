FROM ubuntu:20.04
RUN apt-get update && \
    apt-get -y install gcc mono-mcs && \
    rm -rf /var/lib/apt/lists/
RUN apt-get update && apt-get -y install bsdmainutils
RUN apt-get update && apt-get -y install hexyl
RUN apt-get update && apt-get -y install libcurl4-openssl-dev

COPY . /usr/src/myapp
WORKDIR /usr/src/myapp
RUN gcc -o u udp_server.c -pthread -lcurl
CMD tail -f /dev/null