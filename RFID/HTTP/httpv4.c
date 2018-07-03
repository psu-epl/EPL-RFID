//gcc test2.c -lcurl -o test
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

//Initialize Buffer
void init_string(struct string *s) {
  s->len = 0;
  s->ptr = malloc(s->len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
  }
  s->ptr[0] = '\0';
}

//Callback Function
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

int user_access(char *user, char *station, char *state)
{
  CURL *curl;
  CURLcode res;
  char header[30] =  "User-ID-String: ";
  char header2[30] = "Station-ID: ";
  char header3[30] = "Station-State: ";
  strcat(header, user);
  strcat(header2, station);
  strcat(header3, state);
  struct curl_slist *headers = NULL;

  static const char *postthis = "/api/user-access";
  static const char *pCertFile = "/u/guen/test/LUCCA/test/cert/combined.pem";

  curl = curl_easy_init();
  if(curl) {


    //Append headers to list
    headers = curl_slist_append(headers, header);
    headers = curl_slist_append(headers, header2);
    headers = curl_slist_append(headers, header3);

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    //curl_easy_setopt(curl, CURLOPT_URL, "https://ruby.cecs.pdx.edu:3001");
    curl_easy_setopt(curl, CURLOPT_URL, "http://ptsv2.com/t/xj3ym-1525325338/post");

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postthis);

    //Set String Length
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(postthis));

    //Cert stored PEM coded in file
    curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE, "PEM");

    //Set cert for client authentication
    curl_easy_setopt(curl, CURLOPT_SSLCERT, pCertFile);

    //Res gets return code
    res = curl_easy_perform(curl);
    //Check for errors
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));

    long response_code;
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


//Prints out the time
void heartbeat(char *station)
{
  CURL *curl;
  CURLcode res;
  char *header = "Station-State: Enabled";
  char header2[16] = "Station-ID: ";
  strcat(header2, station);
  struct curl_slist *headers = NULL;

  static const char *postthis = "/api/station-heartbeat";
  static const char *pCertFile = "/u/guen/test/LUCCA/test/cert/combined.pem";

  curl = curl_easy_init();
  if(curl) {

    struct string s;
    init_string(&s);
    headers = curl_slist_append(headers, header);
    headers = curl_slist_append(headers, header2);

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    //curl_easy_setopt(curl, CURLOPT_URL, "https://ruby.cecs.pdx.edu:3001");
    curl_easy_setopt(curl, CURLOPT_URL, "http://ptsv2.com/t/xj3ym-1525325338/post");

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postthis);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEHEADER, &s);

    //Set String Length
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(postthis));

    //Cert stored PEM coded in file
    curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE, "PEM");

    //Set cert for client authentication
    curl_easy_setopt(curl, CURLOPT_SSLCERT, pCertFile);

    //Res gets return code
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
