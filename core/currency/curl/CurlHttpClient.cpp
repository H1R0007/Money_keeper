#include "CurlHttpClient.hpp"
#include <curl/curl.h>

size_t CurlHttpClient::write_callback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t total = size * nmemb;
    output->append((char*)contents, total);
    return total;
}

bool CurlHttpClient::Get(const std::string& url, ResponseCallback callback) {
    CURL* curl = curl_easy_init();
    std::string response;
    bool success = false;

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

        CURLcode res = curl_easy_perform(curl);
        success = (res == CURLE_OK);
        curl_easy_cleanup(curl);
    }

    callback(response, success);
    return success;
}