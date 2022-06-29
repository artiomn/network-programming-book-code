#include <iomanip>
#include <iostream>
#include <string>

#include <curl/curl.h>


static size_t write_cb(void *contents, size_t size, size_t nmemb, void *userp)
{
    reinterpret_cast<std::string*>(userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}


int main(int argc, const char *const argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <url_to_download>" << std::endl;
        return EXIT_FAILURE;
    }

    CURL *curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, argv[1]);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        std::cout << readBuffer << std::endl;
    }

    return EXIT_SUCCESS;
}

