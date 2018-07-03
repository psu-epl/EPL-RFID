//gcc test2.c -lcurl -o test\
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <curl/curl.h>
#define SIZE 200

//Buffer
struct string {
  char *ptr;
  size_t len;
};

/*This function takes in a struct buffer containing a char pointer and allocates memory to it*/
void init_string(struct string *s) {
  s->len = 0;
  s->ptr = malloc(s->len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
  }
  s->ptr[0] = '\0';
}

/*This function takes a pointer to a string and reallocates memory to the buffer
initialized in init_string, and then copies the string over*/
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

/*This function takes in the User-ID, Station-ID and Station State as char strings.
The strings are sent to the server through various functions and a response code is given back.
The function will return an integer that dictates the results of the sending the strings to the server*/
int user_access(char *user, char *station, char *state)
{

  CURL *curl; //Pointer to initiliaze the curl handle
  CURLcode res;

  //Defining the headers in advance to be parsed with what is passed in
  char header[30] =  "User-ID-String: ";
  char header2[30] = "Station-ID: ";
  char header3[30] = "Station-State: ";

  //Parsing headers
  strcat(header, user);
  strcat(header2, station);
  strcat(header3, state);

  //Setting list to NULL at start
  struct curl_slist *headers = NULL;

  static const char *postthis = "/api/user-access";
  static const char *pCertFile = "/u/guen/test/LUCCA/test/cert/combined.pem";

  //Initialize curl handle
  curl = curl_easy_init();
  if(curl) {


    //Append headers to list
    headers = curl_slist_append(headers, header);
    headers = curl_slist_append(headers, header2);
    headers = curl_slist_append(headers, header3);

    //Passes in a pointer to the list of header strings that were concatenated
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    //Passes a char pointer to the URL or server that's being sent data
    //curl_easy_setopt(curl, CURLOPT_URL, "https://ruby.cecs.pdx.edu:3001");
    curl_easy_setopt(curl, CURLOPT_URL, "http://ptsv2.com/t/xj3ym-1525325338/post");

    //Determines if CURL checks authenticity of server's certificate. Set to 1 if API has certificate.
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

    //Checks if server host name provided in certificate matches target server. Set to 1 for same reason.
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    //Sets the information into the body of the HTTP request
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postthis);

    //Set String Length of the request body data being sent
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(postthis));

    //Cert stored PEM coded in file
    curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE, "PEM");

    //Set cert for client authentication
    curl_easy_setopt(curl, CURLOPT_SSLCERT, pCertFile);

    //Res gets success code. Returns 0 if transfer was successful. Non-zero is error.
    res = curl_easy_perform(curl);
    //Check for errors
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));

    long response_code;
    //Gets the response code
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    printf("\n===================\nResponse Code: %ld\n===================", response_code);

    /* Clean up */
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if(response_code == 200)
      return 1;
    else
      return 0;

  }
}

/*This function is provided simply to allow the station modules to send a heartbeat packet to the API. 
It was specified within the CS Team documentation as a method to keep track of stations states.
The API will send back a “200 OK” response code, as the current date and time.*/
void heartbeat(char *station)
{
  CURL *curl; //Pointer to initiliaze the curl handle
  CURLcode res;

  //Defining the headers in advance to be parsed with what is passed in
  char *header = "Station-State: Enabled";
  char header2[16] = "Station-ID: ";
  strcat(header2, station); //Parsing headers

  //Setting list to NULL at start
  struct curl_slist *headers = NULL;

  static const char *postthis = "/api/station-heartbeat";
  static const char *pCertFile = "/u/guen/test/LUCCA/test/cert/combined.pem";

  //Initialize curl handle
  curl = curl_easy_init();
  if(curl) {

    struct string s;

    //Initialize buffer to receive data from server
    init_string(&s);

    //Append headers to list
    headers = curl_slist_append(headers, header);
    headers = curl_slist_append(headers, header2);

    //Passes in a pointer to the list of header strings that were concatenated
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    //Passes a char pointer to the URL or server that's being sent data
    //curl_easy_setopt(curl, CURLOPT_URL, "https://ruby.cecs.pdx.edu:3001");
    curl_easy_setopt(curl, CURLOPT_URL, "http://ptsv2.com/t/xj3ym-1525325338/post");


    //Determines if CURL checks authenticity of server's certificate. Set to 1 if API has certificate.
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    //Checks if server host name provided in certificate matches target server. Set to 1 for same reason.
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    //Sets the information into the body of the HTTP request
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postthis);
    //Sets callback function to receive the strings received from the server
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, writefunc);
    //Writes data from server into the buffer
    curl_easy_setopt(curl, CURLOPT_WRITEHEADER, &s);

    //Set String Length of the request body data being sent
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(postthis));

    //Cert stored PEM coded in file
    curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE, "PEM");

    //Set cert for client authentication
    curl_easy_setopt(curl, CURLOPT_SSLCERT, pCertFile);

    //Res gets success code. Returns 0 if transfer was sucessful.
    res = curl_easy_perform(curl);
    //Check for errors
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));

    //Take out date from headers
    char * token;
    token = strtok(s.ptr, "D");
    token = strtok(NULL, "S");

    printf("\nD%s\n", token);

    // Clean up
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
  }
}

int main(void)
{
    int response;
    char *user = "131027";
    char *station = "printer";
    char *state = "Enabled";


    heartbeat(station);
    response = user_access(user, station, state);
    printf("\n Response: %d\n\n", response);
    return 0;
}
