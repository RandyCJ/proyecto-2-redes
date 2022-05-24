# proyecto-2-redes

To test udp echo server in localhost

`gcc -o server udp_server.c -pthread && ./server`

In another terminal

`nc -u localhost 53`

Then write something and it should return the same message.


In docker

`sudo su`

`docker-compose build`

`docker-compose up`

Go to portainer and get the container IP, then

In another terminal

`nc -u containerIP 53`

Then write something and it should return the same message.
