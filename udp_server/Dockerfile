FROM ubuntu:20.04
RUN apt-get update && \
    apt-get -y install gcc mono-mcs && \
    rm -rf /var/lib/apt/lists/
RUN apt-get update && apt-get -y install bsdmainutils

COPY . /usr/src/myapp
WORKDIR /usr/src/myapp
RUN gcc -o udp_server udp_server.c -pthread
CMD tail -f /dev/null