/*
 * Simple HTTP POST using the easy interface
 */
extern "C"
{
#include <curl/curl.h>
}

#include <iostream>


int main(void)
{
    CURL *curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_ALL);

    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, "https://httpbin.org/post");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "name=daniel&project=curl");

        res = curl_easy_perform(curl);

        if (res != CURLE_OK)
        {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res);
        }

        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    return EXIT_SUCCESS;
}
