#include <stdint.h>
#include <stdio.h>

typedef int32_t i32;
typedef int64_t i64;

#include <curl/curl.h>

typedef CURL     Curl;
typedef CURLcode CurlCode;

#define OK 0

i32 main(void) {
    Curl* curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://pie.dev/delay/1");
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_USE_SSL, (i64)CURLUSESSL_ALL);

        const CurlCode result = curl_easy_perform(curl);
        if (result != CURLE_OK) {
            fprintf(stderr,
                    "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(result));
        }

        curl_easy_cleanup(curl);
    }
    return OK;
}
