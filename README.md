# proyecto-2-redes

To test udp echo server

`gcc -o server udp_server.c -pthread && ./server`

In another terminal

`nc -u localhost 53`

Then write something and it should return the same message.
