#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <json-c/json.h>
#include <regex.h>
#include "dns_interceptor.h"
#include "base64.c"

struct string {
  char *ptr;
  size_t len;
};

void init_string(struct string *s) {
  s->len = 0;
  s->ptr = malloc(s->len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
  }
  s->ptr[0] = '\0';
}

size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s)
{
  size_t new_len = s->len + size*nmemb;
  s->ptr = realloc(s->ptr, new_len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "realloc() failed\n");
    exit(EXIT_FAILURE);
  }
  memcpy(s->ptr+s->len, ptr, size*nmemb);
  s->ptr[new_len] = '\0';
  s->len = new_len;

  return size*nmemb;
}

unsigned int dotted_decimal_to_int(char ip[]){
 
    // char is exactly 1 byte
    unsigned char bytes[4] = {0};
    
    sscanf(ip, "%hhd.%hhd.%hhd.%hhd", &bytes[3], &bytes[2], &bytes[1], &bytes[0]);
    
    // set 1 byte at a time by left shifting (<<) and ORing (|)
    return bytes[0] | bytes[1] << 8 | bytes[2] << 16 | bytes[3] << 24;

}

void *write_file(char *path, unsigned char buf[], int bytes){
	FILE *fp;
	fp = fopen("h1.txt", "w+");

	if (!fp) {
        printf("Unable to open/"
               "detect file(s)\n");
        return NULL;
    }
	
	fwrite(buf, 1, bytes, fp);
	
	fclose(fp);
}

void get_header_hostname(unsigned char buf[], struct hostname_rep *hostname_struct){

	char hostname[BUFSIZE] = "";
	char char_tmp[2];
	int start = 12;
	int amount = (int)buf[start];
	unsigned char binary_hostname[BUFSIZE];

	binary_hostname[0] = buf[start];
	int j = 1;

	for (int i = start+1; buf[i] != 0; i++){
		binary_hostname[j] = buf[i];
		j++;
		if (i == start + amount + 1){
			start = start + amount + 1;
			amount = (int)buf[i];
			strcat(hostname, ".");
			continue;
		}
		sprintf(char_tmp, "%c", buf[i]);
		strcat(hostname, char_tmp);
	}

	memcpy(hostname_struct->ascii_hostname, hostname, j);
	memcpy(hostname_struct->header_hostname, binary_hostname, j);
}

void *get_response(unsigned char buf[BUFSIZE], int read_bytes, struct response_data *response){
	
	write_file("h1.txt", buf, read_bytes);

	struct hostname_rep hostname;
	get_header_hostname(buf, &hostname);

	CURL *curl;
	CURLcode res;
	

	struct json_object *parsed_json;
	struct json_object *ip;
	struct json_object *index;
	/* In windows, this will init the winsock stuff */
	curl_global_init(CURL_GLOBAL_ALL);
	/* get a curl handle */
	curl = curl_easy_init();
	char *tmp_ip = "";
	int flag = 0;
	char elastic_ptr[BUFSIZE];
	unsigned int elastic_ip_address;
	
	if(curl) {
	 	struct string elastic;
	 	init_string(&elastic);
	 	/* First set the URL that is about to receive our POST. This URL can
	 	just as well be a https:// URL if that is what should receive the
	 	data. */
	 	char get_request[BUFSIZE] = "10.5.0.5:9200/zones/host/";
	 	strcat(get_request, hostname.ascii_hostname);
	 	strcat(get_request, "/_source");
	 	curl_easy_setopt(curl, CURLOPT_URL, get_request);
	 	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
	 	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &elastic);
	 	/* Perform the request, res will get the return code */
	 	res = curl_easy_perform(curl);
	 	
		 
		regex_t reegex;
		
    	int ip_found = regcomp( &reegex, "[0-9]?[0-9]?[0-9][.][0-9]?[0-9]?[0-9][.][0-9]?[0-9]?[0-9][.][0-9]?[0-9]?[0-9]", REG_EXTENDED);
    	strcpy(elastic_ptr,elastic.ptr);
		ip_found = regexec( &reegex, elastic_ptr, 0, NULL, 0); 
		parsed_json = json_tokener_parse(elastic.ptr);
		
		if (ip_found == 0 )
		{
			
			unsigned int tmp_int_ips[BUFSIZE];
			int tmp_index;
			flag = 0;
			json_object_object_get_ex(parsed_json, "IP", &ip);
			tmp_ip = json_object_get_string(ip);
			char *token = strtok(tmp_ip, ",");
			tmp_int_ips[0] = dotted_decimal_to_int(token);
			int count = 1;

			while (token != NULL)
			{
				token = strtok(NULL, ",");
				if (token == NULL)
				{
					count ++;
					break;
				}
				tmp_int_ips[count] = dotted_decimal_to_int(token);
				count ++;
			}
			
			
			json_object_object_get_ex(parsed_json, "index", &index);
			tmp_index = atoi(json_object_get_string(index));
			
			if (tmp_index > count-2 )
			{
				tmp_index = 0;
			}
			
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

				struct curl_slist *list = NULL;
				list = curl_slist_append(list, "Content-Type: application/json");

				char json_update[BUFSIZE] = "{\"doc\": {\"index\": ";
				char index_str[BUFSIZE];
				sprintf(index_str,"%d",tmp_index+1);
				strcat(json_update, index_str );
				strcat(json_update, " } }");
				char update_request[BUFSIZE] = "10.5.0.5:9200/zones/host/";
				strcat(update_request, hostname.ascii_hostname);
	 			strcat(update_request, "/_update");
				
				curl_easy_setopt(curl, CURLOPT_URL, update_request);
				
				/* Now specify the POST data */
				curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_update);
				curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
				
				/* Perform the request, res will get the return code */
				res = curl_easy_perform(curl);
	
				/* Check for errors */
				if(res != CURLE_OK)
				fprintf(stderr, "curl_easy_perform() failed: %s\n",
						curl_easy_strerror(res));
				

				/* always cleanup */
				curl_easy_cleanup(curl);
				curl_slist_free_all(list); /* free the list again */
			}
			elastic_ip_address = tmp_int_ips[tmp_index];
		}
		else{
			
			flag = 1;
		}
	}

	if (flag == 0)
	{
		unsigned char response_query[BUFSIZE];
		memcpy(response_query, buf, read_bytes);

		response_query[2] = 129;
		response_query[3] = 128;
		response_query[7] = 1;

		int count = read_bytes;

		// NAME
		for (int i = 0; i < strlen(hostname.header_hostname); i++)
		{
			response_query[count] = hostname.header_hostname[i];
			count++;
		}
		response_query[count++] = 0;

		// TYPE (A) = ipv4 address
		response_query[count++] = 0;
		response_query[count++] = 1;
		// CLASS (IN) = Internet
		response_query[count++] = 0;
		response_query[count++] = 1;
		// TTL (colocar TTL de elastic search, o no?)
		response_query[count++] = 0;
		response_query[count++] = 0;
		response_query[count++] = 0;
		response_query[count++] = 5;
		// RDLENGTH
		response_query[count++] = 0;
		response_query[count++] = 4;
		// RDATA 
		response_query[count++] = elastic_ip_address >> 24;
		response_query[count++] = elastic_ip_address >> 16;
		response_query[count++] = elastic_ip_address >> 8;
		response_query[count++] = elastic_ip_address;
		response_query[count] = 0;
			
        memcpy(response->response, response_query, count);
        response->bytes_response = count;
		return NULL;
	}


	struct DNS_HEADER *dns = NULL;
	dns = (struct DNS_HEADER *)(char*)buf;
	char       *enc;
	unsigned char       *out;
	size_t      out_len;
	if (dns->qr == 0 && dns->opcode == 0 )
	{

		char data[BUFSIZE] = "{\"dns\": \"8.8.8.8\", \"port\": 53, \"data\":\"";

		enc = b64_encode((const unsigned char *)buf, read_bytes);
		strcat(data, enc);
		strcat(data, "\"}");
		CURL *curl2;
		CURLcode res2;
		
		/* In windows, this will init the winsock stuff */
		curl_global_init(CURL_GLOBAL_ALL);

		/* get a curl handle */
		curl2 = curl_easy_init();
		struct json_object *parsed_json;
		struct json_object *answer;
		if(curl2) {
			/* First set the URL that is about to receive our POST. This URL can
			just as well be a https:// URL if that is what should receive the
			data. */
			struct string s;
			init_string(&s);
			curl_easy_setopt(curl2, CURLOPT_URL, "http://10.5.0.6:443/api/dns_resolver");
			/* Now specify the POST data */
			curl_easy_setopt(curl2, CURLOPT_POSTFIELDS, data);
			curl_easy_setopt(curl2, CURLOPT_WRITEFUNCTION, writefunc);
			curl_easy_setopt(curl2, CURLOPT_WRITEDATA, &s);

			/* Perform the request, res will get the return code */
			res2 = curl_easy_perform(curl2);
			parsed_json = json_tokener_parse(s.ptr);
			json_object_object_get_ex(parsed_json, "answer", &answer);

			free(s.ptr);
			
			/* Check for errors */
			if(res2 != CURLE_OK)
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
					curl_easy_strerror(res2));
			

			/* always cleanup */
			curl_easy_cleanup(curl2);
		}
		curl_global_cleanup();
    	
		
		out_len = b64_decoded_size(json_object_get_string(answer));
		out = malloc(out_len);
		b64_decode(json_object_get_string(answer), (unsigned char *)out, out_len);
		
		write_file("r1.txt", out, out_len);

	}
	else printf("Not implemented\n");

    memcpy(response->response, out, out_len);
    response->bytes_response = out_len;
	free(out);
    return NULL;
}
