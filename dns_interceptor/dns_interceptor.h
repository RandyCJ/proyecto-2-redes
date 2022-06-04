
#define BUFSIZE 1024
#define DNS_API "http://10.5.0.6:443/api/dns_resolver"
#define ELASTIC_API "10.5.0.5:9200/zones/host/"
#define VALID_IP_REGEX "[0-9]?[0-9]?[0-9][.][0-9]?[0-9]?[0-9][.][0-9]?[0-9]?[0-9][.][0-9]?[0-9]?[0-9]"
#define VALID_INDEX_REGEX "\"index\":"
#define DNS_API_DATA "{\"dns\": \"8.8.8.8\", \"port\": 53, \"data\":\""

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

struct response_data {
    unsigned char response[BUFSIZE];
    int bytes_response;
};

struct hostname_rep {
    unsigned char ascii_hostname[BUFSIZE];
    unsigned char header_hostname[BUFSIZE];
};

struct elastic_data {
    unsigned int ip;
    int index;
};

unsigned int dotted_decimal_to_int(char ip[]);
/*
-----------------------------------------------------------------------
    dotted_decimal_to_int
    Input: string with the ip address
    Output: unsigned int
    Functioning: Convert a ip string ("10.0.0.0") to unsigned int
-----------------------------------------------------------------------
*/

void *write_file(char *path, unsigned char buf[], int bytes);
/*
-----------------------------------------------------------------------
    write_file
    Input: A pointer string with the path, unsigned string to write, 
    int with the lenght to write
    Output: Void
    Functioning: Write a string in a file, create file if it doesn't exist
-----------------------------------------------------------------------
*/

void get_header_hostname(unsigned char buf[], struct hostname_rep *);
/*
-----------------------------------------------------------------------
    get_header_hostname
    Input: A unsigned string with the DNS package , empty struct hostnames 
    Output: Void
    Functioning: Extract hostname (in ASCII and in DNS format ) 
    that have DNS package
-----------------------------------------------------------------------
*/
void get_elastic_ip_and_index(struct elastic_data *, char *elastic_ips);
/*
-----------------------------------------------------------------------
    get_elastic_ip_and_index
    Input: A empty struct to save ip and index, pointer string with ips 
    Output: Void
    Functioning: Extract the index of elasticsearch and verify the good
    function of the round robbin and stores ip and index in the struct
    
-----------------------------------------------------------------------
*/
void build_dns_response(unsigned char response_query[], int count, unsigned char hostname[], 
						unsigned int elastic_ip_address, struct response_data *);
/*
-----------------------------------------------------------------------
    build_dns_response
    Input: A unsigned string that have buffer of response, int that have
    lenght of the buffer, unsigned string with the unedited hostnmae,unsigned
    int the ip address of elasticsearch, empty struct to store DNS response
    Output: Void
    Functioning: Build DNS response byte per byte
    
-----------------------------------------------------------------------
*/
void *get_response(unsigned char buf[BUFSIZE], int read_bytes, struct response_data *);

/*
-----------------------------------------------------------------------
    get_response
    Input: A unsigned string that have buffer of request DNS, int that have
    lenght of the buffer, empty struct to store DNS response
    Output: Void
    Functioning: Main function that consults elasticsearch by hostname, if
    not found, sends the package to DNS API, and stores the DNS response in
    the response_data struct
    
-----------------------------------------------------------------------
*/