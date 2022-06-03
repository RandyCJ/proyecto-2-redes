#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <json-c/json.h>
#include <regex.h>
#include "dns_interceptor.h"
#include "base64.c"
#include "http_client.c"

unsigned int dotted_decimal_to_int(char ip[]){
 
    // char is exactly 1 byte
    unsigned char bytes[4] = {0};
    
    sscanf(ip, "%hhd.%hhd.%hhd.%hhd", &bytes[3], &bytes[2], &bytes[1], &bytes[0]);
    
    // set 1 byte at a time by left shifting (<<) and ORing (|)
    return bytes[0] | bytes[1] << 8 | bytes[2] << 16 | bytes[3] << 24;

}

void *write_file(char *path, unsigned char buf[], int bytes){
	FILE *fp;
	fp = fopen(path, "w+");

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

void get_elastic_ip_and_index(struct elastic_data *elstc_data, char *elastic_ips){
	struct json_object *parsed_json;
	struct json_object *ip;
	char *tmp_ip = "";
		
	parsed_json = json_tokener_parse(elastic_ips);
	
	unsigned int tmp_int_ips[BUFSIZE];
	int tmp_index;
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
	
	struct json_object *index;
	json_object_object_get_ex(parsed_json, "index", &index);
	tmp_index = atoi(json_object_get_string(index));
	
	if (tmp_index > count-2 )
	{
		tmp_index = 0;
	}
	elstc_data->ip = tmp_int_ips[tmp_index];
	elstc_data->index = tmp_index;
}

void build_dns_response(unsigned char response_query[], int count, unsigned char hostname[], 
						unsigned int elastic_ip_address, struct response_data *response){
	response_query[2] = 129;
	response_query[3] = 128;
	response_query[7] = 1;

	// NAME
	for (int i = 0; i < strlen(hostname); i++)
	{
		response_query[count] = hostname[i];
		count++;
	}
	response_query[count++] = 0;

	// TYPE (A) = ipv4 address
	response_query[count++] = 0;
	response_query[count++] = 1;
	// CLASS (IN) = Internet
	response_query[count++] = 0;
	response_query[count++] = 1;
	// TTL 
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
}

void *get_response(unsigned char buf[BUFSIZE], int read_bytes, struct response_data *response){
	
	write_file("h1.txt", buf, read_bytes);

	// Get hostname from buffer
	struct hostname_rep hostname;
	get_header_hostname(buf, &hostname);

	// ElasticSearch hostname request
	struct string elastic;
	char get_request[BUFSIZE] = ELASTIC_API;
	strcat(get_request, hostname.ascii_hostname);
	strcat(get_request, "/_source");
	request_response(&elastic, get_request, "", 1);

	// Check if hostname exist with valid IPs
	char elastic_ptr[BUFSIZE];
	strcpy(elastic_ptr, elastic.ptr);
	regex_t reegex;
	int ip_found = regcomp( &reegex, VALID_IP_REGEX, REG_EXTENDED);
	ip_found = regexec( &reegex, elastic_ptr, 0, NULL, 0);

	if (ip_found == 0 ){ // If valid IP

		// get ip and index from elastic
		struct elastic_data elstc_data;
		get_elastic_ip_and_index(&elstc_data, elastic.ptr);
		
		// update index in elastic register
		char json_update[BUFSIZE] = "{\"doc\": {\"index\": ";
		char index_str[BUFSIZE];
		sprintf(index_str, "%d", elstc_data.index+1);
		strcat(json_update, index_str );
		strcat(json_update, " } }");
		char update_request[BUFSIZE] = ELASTIC_API;
		strcat(update_request, hostname.ascii_hostname);
		strcat(update_request, "/_update");
		request_response(&elastic, update_request, json_update, 0);

		// build dns response
		build_dns_response(buf, read_bytes, hostname.header_hostname, elstc_data.ip, response);

		return NULL;
	
	}

	// When IP not found in elastic, request to DNS API
	struct DNS_HEADER *dns = NULL;
	dns = (struct DNS_HEADER *)(char*)buf;
	char *enc;
	unsigned char *out;
	size_t out_len;
	if (dns->qr == 0 && dns->opcode == 0) // QR and OPCODE must be equal to 0
	{
		char data[BUFSIZE] = DNS_API_DATA;

		// encoding buffer to base64 and building POST data
		enc = b64_encode((const unsigned char *)buf, read_bytes);
		strcat(data, enc);
		strcat(data, "\"}");

		// https post request
		struct string s;
		request_response(&s, DNS_API, data, 1);

		// obtaining request
		struct json_object *parsed_json;
		struct json_object *answer;
		parsed_json = json_tokener_parse(s.ptr);
		json_object_object_get_ex(parsed_json, "answer", &answer);

    	
		out_len = b64_decoded_size(json_object_get_string(answer)); // bytes length
		out = malloc(out_len); // DNS API response

		// Decoding DNS API response
		b64_decode(json_object_get_string(answer), (unsigned char *)out, out_len);
		
		free(s.ptr);
		write_file("r1.txt", out, out_len);

	}
	else printf("Not implemented\n");

	// Storing response and its length in struct
    memcpy(response->response, out, out_len);
    response->bytes_response = out_len;
	free(out);
    return NULL;
}
