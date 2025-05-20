#pragma once
#include <string>
#include <unordered_map>
#include <mutex>
#include <functional>

class CurrencyConverter {
public:
    void update_rates(std::function<void(bool)> callback);
    double convert(double amount, const std::string& from, const std::string& to = "RUB") const;

    bool is_currency_supported(const std::string& currency_code) const {
        //std::lock_guard<std::mutex> lock(rates_mutex_);
        return rates_.find(currency_code) != rates_.end();
    }

    void save_rates_to_file(const std::string& path);
    bool load_rates_from_file(const std::string& path);

    void set_rates(const std::unordered_map<std::string, double>& new_rates) {
        std::lock_guard<std::mutex> lock(rates_mutex_);
        rates_ = new_rates;
    }

private:
    std::unordered_map<std::string, double> rates_;
    mutable std::mutex rates_mutex_;
};
