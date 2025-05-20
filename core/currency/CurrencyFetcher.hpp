#pragma once
#include <string>
#include <unordered_map>
#include <functional>

class CurrencyFetcher {
public:
    using RatesCallback = std::function<void(const std::unordered_map<std::string, double>&)>;

    void fetch_rates(RatesCallback callback);

private:
    bool parse_json(const std::string& json, std::unordered_map<std::string, double>& rates);
};
