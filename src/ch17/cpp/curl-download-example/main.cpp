extern "C"
{
#include <curl/curl.h>
}

#include <iomanip>
#include <iostream>
#include <string>


static size_t write_cb(void *contents, size_t size, size_t nmemb, void *userp)
{
    auto res = size * nmemb;
    reinterpret_cast<std::string *>(userp)->append(static_cast<char *>(contents), res);
    return res;
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
