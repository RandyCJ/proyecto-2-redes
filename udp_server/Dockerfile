FROM gcc:9.4.0
COPY . /usr/src/myapp
WORKDIR /usr/src/myapp
RUN gcc -o udp_server udp_server.c -pthread
CMD ["./udp_server"]