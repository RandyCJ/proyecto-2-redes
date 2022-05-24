/* 
 * Code taken from  @miekg at https://gist.github.com/miekg/a61d55a8ec6560ad6c4a2747b21e6128
 * 
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>

#define BUFSIZE 1024

struct args {
    struct sockaddr_in clientaddr;
    char buf[1024];
	int n;
	int socket;
	int client_len;
};

pthread_t threads[15];

void error(char *msg)
{
	perror(msg);
	exit(1);
}

void *handle_request(void *t_args){
	
	struct hostent *hostp; /* client host info */
	char *hostaddrp;	/* dotted decimal host addr string */

	hostp = gethostbyaddr((const char *)&((struct args*)t_args)->clientaddr.sin_addr.s_addr,
					sizeof(((struct args*)t_args)->clientaddr.sin_addr.s_addr),
					AF_INET);
	if (hostp == NULL)
		error("ERROR on gethostbyaddr");
	hostaddrp = inet_ntoa(((struct args*)t_args)->clientaddr.sin_addr);
	if (hostaddrp == NULL)
		error("ERROR on inet_ntoa\n");
	printf("server received %d bytes\n", ((struct args*)t_args)->n);

	/* 
		* sendto: echo the input back to the client 
	*/
	int n = sendto(((struct args*)t_args)->socket, ((struct args*)t_args)->buf, ((struct args*)t_args)->n, 0,
			(struct sockaddr *)&((struct args*)t_args)->clientaddr, ((struct args*)t_args)->client_len);
	if (n < 0)
		error("ERROR in sendto");
}

int main()
{
	int sockfd;		/* socket */
	int portno;		/* port to listen on */
	int clientlen;		/* byte size of client's address */
	struct sockaddr_in serveraddr;	/* server's addr */
	struct sockaddr_in clientaddr;	/* client addr */
	char buf[BUFSIZE];		/* message buf */
	int optval;		/* flag value for setsockopt */
	int n;			/* message byte size */

	/* 
	 * check command line arguments 
	 */
	portno = 53;

	/* 
	 * socket: create the parent socket 
	 */
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");

	/* setsockopt: Handy debugging trick that lets 
	 * us rerun the server immediately after we kill it; 
	 * otherwise we have to wait about 20 secs. 
	 * Eliminates "ERROR on binding: Address already in use" error. 
	 */
	optval = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
		   (const void *)&optval, sizeof(int));

	/*
	 * build the server's Internet address
	 */
	bzero((char *)&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons((unsigned short)portno);

	/* 
	 * bind: associate the parent socket with a port 
	 */
	if (bind(sockfd, (struct sockaddr *)&serveraddr,
		 sizeof(serveraddr)) < 0)
		error("ERROR on binding");

	/* 
	 * main loop: wait for a datagram, then echo it
	 */
	clientlen = sizeof(clientaddr);
	int i = 0;
	while (1) {

		/*
		 * recvfrom: receive a UDP datagram from a client
		 */
		n = recvfrom(sockfd, buf, BUFSIZE, 0,
			     (struct sockaddr *)&clientaddr, &clientlen);
		if (n < 0)
			error("ERROR in recvfrom");
		
		struct args *thread_args = (struct args *)malloc(sizeof(struct args));
		thread_args->clientaddr = clientaddr;
		strcpy(thread_args->buf, buf);
		thread_args->n = n;
		thread_args->socket = sockfd;
		thread_args->client_len = clientlen;

		if (pthread_create(&threads[i++], NULL, handle_request, (void *) thread_args) != 0){
			printf("Failed to create thread\n");
		}

		if (i >= 15) {
            // Update i
            i = 0;
 
            while (i < 15) {
                // Suspend execution of
                // the calling thread
                // until the target
                // thread terminates
                pthread_join(threads[i++],
                             NULL);
            }
 
            // Update i
            i = 0;
        }

	}
}