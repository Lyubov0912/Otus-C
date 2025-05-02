#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include "cJSON.h"

struct memory {
  char *response;
  size_t size;
};

static size_t cb(void *data, size_t size, size_t nmemb, void *clientp) {
  size_t realsize = size * nmemb;
  struct memory *mem = (struct memory *)clientp;

  char *ptr = realloc(mem->response, mem->size + realsize + 1);
  if(ptr == NULL)
    return 0;  /* out of memory! */

  mem->response = ptr;
  memcpy(&(mem->response[mem->size]), data, realsize);
  mem->size += realsize;
  mem->response[mem->size] = 0;

  return realsize;
}

void abort_on_error_check(CURLcode code, void(*callback)(void *), void *opaque) {
  if (CURLE_OK != code) {
    fprintf(stderr, "CURL Error: %s\n", curl_easy_strerror(code));
    callback(opaque);
    exit(code);
  }
}

#define abort_on_error(code) \
abort_on_error_check(code, curl_easy_cleanup, p_curl)

unsigned long __stack_chk_guard;
void __stack_chk_guard_setup(void)
{
     __stack_chk_guard = 0xBAAAAAAD;//provide some magic numbers
}

void __stack_chk_fail(void)
{
 /* Error message */
}// will be called when guard variable is corrupted

void list_children(cJSON* json) {
    while (json != 0) {
        printf("%d %s\n", json->type, json->string);
        json = json->next;
    }
}

int main(int argc, char *argv[]) {

    char url[100] = "https://wttr.in/";
    if (argc > 1)
    {
        char city[15];
        strcpy(city, argv[1]);
        strcat(url, city);
        strcat(url, "?format=j1");
    }
    else
    {
        printf("City not entered");
        return -1;
    }
  CURL *p_curl;

  struct memory mem = { 0 };

  p_curl = curl_easy_init();
  if (NULL == p_curl) {
    (void) fprintf(stderr, "CURL init failed\n");
  }

  abort_on_error(curl_easy_setopt(p_curl, CURLOPT_URL, url));
  abort_on_error(curl_easy_setopt(p_curl, CURLOPT_SSL_VERIFYPEER, 0L));
  abort_on_error(curl_easy_setopt(p_curl, CURLOPT_WRITEDATA, &mem));
  abort_on_error(curl_easy_setopt(p_curl, CURLOPT_WRITEFUNCTION, cb));
  abort_on_error(curl_easy_perform(p_curl));

  int response_code;
  abort_on_error(curl_easy_getinfo(p_curl, CURLINFO_RESPONSE_CODE, &response_code));
  if (response_code != 200) {
      printf("No such city \"%s\" found!\n", argv[1]);
      return -1;
  }

  cJSON* root = cJSON_Parse(mem.response);

  cJSON* curr_cond_array = cJSON_GetObjectItem(root, "current_condition");
  cJSON* curr_cond_first = cJSON_GetArrayItem(curr_cond_array, 0);
  cJSON* wind_dir_obj = cJSON_GetObjectItem(curr_cond_first, "winddir16Point");

  cJSON* weather_desc_array = cJSON_GetObjectItem(curr_cond_first, "weatherDesc");
  cJSON* weather_desc_first = cJSON_GetArrayItem(weather_desc_array, 0);
  cJSON* weather_desc_obj = cJSON_GetObjectItem(weather_desc_first, "value");

  cJSON* weather_array = cJSON_GetObjectItem(root, "weather");
  cJSON* weather_first = cJSON_GetArrayItem(weather_array, 0);
  cJSON* min_temp_c_obj = cJSON_GetObjectItem(weather_first, "mintempC");
  cJSON* max_temp_c_obj = cJSON_GetObjectItem(weather_first, "maxtempC");

  printf("Wind direction: %s\n"
         "Description: %s\n"
         "Temp: %s-%s\n",
         wind_dir_obj->valuestring,
         weather_desc_obj->valuestring,
         min_temp_c_obj->valuestring,
         max_temp_c_obj->valuestring);

  free(mem.response);
  curl_easy_cleanup(p_curl);
}
