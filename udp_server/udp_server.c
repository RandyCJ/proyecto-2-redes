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
#include <sys/stat.h>
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h> /* struct hostent, gethostbyname */
#include <curl/curl.h>

#define BUFSIZE 1024

const char b64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int b64invs[] = { 62, -1, -1, -1, 63, 52, 53, 54, 55, 56, 57, 58,
	59, 60, 61, -1, -1, -1, -1, -1, -1, -1, 0, 1, 2, 3, 4, 5,
	6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
	21, 22, 23, 24, 25, -1, -1, -1, -1, -1, -1, 26, 27, 28,
	29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
	43, 44, 45, 46, 47, 48, 49, 50, 51 };


struct DNS_HEADER
{
    unsigned short id; // identification number
 
    unsigned char rd :1; // recursion desired
    unsigned char tc :1; // truncated message
    unsigned char aa :1; // authoritive answer
    unsigned char opcode :4; // purpose of message
    unsigned char qr :1; // query/response flag
 
    unsigned char rcode :4; // response code
    unsigned char cd :1; // checking disabled
    unsigned char ad :1; // authenticated data
    unsigned char z :1; // its z! reserved
    unsigned char ra :1; // recursion available
 
    unsigned short q_count; // number of question entries
    unsigned short ans_count; // number of answer entries
    unsigned short auth_count; // number of authority entries
    unsigned short add_count; // number of resource entries
};

struct args {
    struct sockaddr_in clientaddr;
    unsigned char buf[BUFSIZE];
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

/*
Base 64 encoding functions taken from https://nachtimwald.com/2017/11/18/base64-encode-and-decode-in-c/
*/

size_t b64_encoded_size(size_t inlen)
{
	size_t ret;

	ret = inlen;
	if (inlen % 3 != 0)
		ret += 3 - (inlen % 3);
	ret /= 3;
	ret *= 4;

	return ret;
}
char *b64_encode(const unsigned char *in, size_t len)
{
	char   *out;
	size_t  elen;
	size_t  i;
	size_t  j;
	size_t  v;

	if (in == NULL || len == 0)
		return NULL;

	elen = b64_encoded_size(len);
	out  = malloc(elen+1);
	out[elen] = '\0';

	for (i=0, j=0; i<len; i+=3, j+=4) {
		v = in[i];
		v = i+1 < len ? v << 8 | in[i+1] : v << 8;
		v = i+2 < len ? v << 8 | in[i+2] : v << 8;

		out[j]   = b64chars[(v >> 18) & 0x3F];
		out[j+1] = b64chars[(v >> 12) & 0x3F];
		if (i+1 < len) {
			out[j+2] = b64chars[(v >> 6) & 0x3F];
		} else {
			out[j+2] = '=';
		}
		if (i+2 < len) {
			out[j+3] = b64chars[v & 0x3F];
		} else {
			out[j+3] = '=';
		}
	}

	return out;
}

size_t b64_decoded_size(const char *in)
{
	size_t len;
	size_t ret;
	size_t i;

	if (in == NULL)
		return 0;

	len = strlen(in);
	ret = len / 4 * 3;

	for (i=len; i-->0; ) {
		if (in[i] == '=') {
			ret--;
		} else {
			break;
		}
	}

	return ret;
}

void b64_generate_decode_table()
{
	int    inv[80];
	size_t i;

	memset(inv, -1, sizeof(inv));
	for (i=0; i<sizeof(b64chars)-1; i++) {
		inv[b64chars[i]-43] = i;
	}
}

int b64_isvalidchar(char c)
{
	if (c >= '0' && c <= '9')
		return 1;
	if (c >= 'A' && c <= 'Z')
		return 1;
	if (c >= 'a' && c <= 'z')
		return 1;
	if (c == '+' || c == '/' || c == '=')
		return 1;
	return 0;
}

void *handle_request(void *t_args){
	
	char *data;
	char       *enc;
	char       *out;
	size_t      out_len;
	
	struct DNS_HEADER *dns = NULL;
	dns = (struct DNS_HEADER *)(char*)((struct args*)t_args)->buf;
	
	for (int i = 13; i < ((struct args*)t_args)->n-5; i++)
	{				
		printf(" %i : %c : %u \n", i, ((struct args*)t_args)->buf[i], ((struct args*)t_args)->buf[i]);
	}
	
	FILE *fp1;
	fp1 = fopen("text.txt", "w+");

	if (!fp1) {
        printf("Unable to open/"
               "detect file(s)\n");
        return NULL;
    }
	
	fwrite(((struct args*)t_args)->buf, 1, ((struct args*)t_args)->n, fp1);
	
	fclose(fp1);

	if (dns->qr == 0 && dns->opcode == 0 )
	{
		char data[BUFSIZE] = "{\"dns\": \"8.8.8.8\", \"port\": 53, \"data\":\"";

		enc = b64_encode((const unsigned char *)((struct args*)t_args)->buf, ((struct args*)t_args)->n);
		printf("encoded: '%s'\n", enc);
		strcat(data, enc);
		strcat(data, "\"}");
		printf("DATA: %s\n", data);
		CURL *curl;
		CURLcode res;
		
		/* In windows, this will init the winsock stuff */
		curl_global_init(CURL_GLOBAL_ALL);
		
		/* get a curl handle */
		curl = curl_easy_init();
		if(curl) {
			/* First set the URL that is about to receive our POST. This URL can
			just as well be a https:// URL if that is what should receive the
			data. */
			curl_easy_setopt(curl, CURLOPT_URL, "10.5.0.6:443/api/dns_resolver");
			/* Now specify the POST data */
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
		
			/* Perform the request, res will get the return code */
			res = curl_easy_perform(curl);
			/* Check for errors */
			if(res != CURLE_OK)
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
					curl_easy_strerror(res));
		
			/* always cleanup */
			curl_easy_cleanup(curl);
		}
		curl_global_cleanup();
	}
	else printf("Not implemented\n");
	
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
	
	//printf("server received %d bytes\n", ((struct args*)t_args)->n);

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
	unsigned char buf[BUFSIZE];		/* message buf */
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
		memcpy(thread_args->buf, buf, n);
		thread_args->n = n;
		thread_args->socket = sockfd;
		thread_args->client_len = clientlen;

	
		if (pthread_create(&threads[i], NULL, handle_request, (void *) thread_args) != 0){
			printf("Failed to create thread\n");
		}
		i++;

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