#pragma once
#include <string>
#include <functional>

class CurlHttpClient {
public:
    using ResponseCallback = std::function<void(const std::string&, bool)>;

    static bool Get(const std::string& url, ResponseCallback callback);

private:
    static size_t write_callback(void* contents, size_t size, size_t nmemb, std::string* output);
};
