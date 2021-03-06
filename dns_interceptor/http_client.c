#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

struct string {
    char *ptr;
    size_t len;
};

void init_string(struct string *s) {
/*
-----------------------------------------------------------------------
    init_string
    Input: A empty struct to store the HTTP response
    Output: Void
    Functioning: Initialize the string struct
    
-----------------------------------------------------------------------
*/
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
/*
-----------------------------------------------------------------------
    writefunc
    Input: A pointer to save in a struct, size_t with size, size_t with 
    number of members to write, empty struct to store the HTTP response
    Output: Size_t
    Functioning: Write in the string struct
    
-----------------------------------------------------------------------
*/
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

void request_response(struct string *response, unsigned char url[], unsigned char data[], int flag){
/*
-----------------------------------------------------------------------
    request_response
    Input: A empty struct to store response, unsigned string with url,
    unsigned string with POST data, int that have flag to decide whether 
    to put headers or not
    Output: void
    Functioning: Send HTTP request (POST, GET) and stores response in a struct
    
-----------------------------------------------------------------------
*/   
    CURL *curl;
    CURLcode res;
    struct curl_slist *list = NULL;
    curl_global_init(CURL_GLOBAL_ALL);
    /* get a curl handle */
    curl = curl_easy_init();

    init_string(response);

  	if(curl) {

      curl_easy_setopt(curl, CURLOPT_URL, url);
      /*SSL Certificate*/
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
      if (strcmp(data, "") != 0){

        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);

      }
      if (flag == 0){
				list = curl_slist_append(list, "Content-Type: application/json");
				curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
      }
      
    
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);
      /* Perform the request, res will get the return code */
      res = curl_easy_perform(curl);

      if(res != CURLE_OK)
				fprintf(stderr, "curl_easy_perform() failed: %s\n",
						curl_easy_strerror(res));
				

				/* always cleanup */
				curl_easy_cleanup(curl);
				curl_slist_free_all(list); /* free the list again */
    }
    curl_global_cleanup();

}