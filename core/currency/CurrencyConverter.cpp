#include "CurrencyConverter.hpp"
#include "CurrencyFetcher.hpp"
#include <../libs/json.hpp>
#include <fstream>
#include <filesystem>
#include <iostream>

void CurrencyConverter::update_rates(std::function<void(bool)> callback) {
    CurrencyFetcher fetcher;
    fetcher.fetch_rates([this, callback](const auto& new_rates) {
        std::lock_guard<std::mutex> lock(rates_mutex_);
        rates_ = new_rates;
        std::cout << "[DEBUG] Загружено курсов: " << rates_.size() << "\n";
        callback(!new_rates.empty());
        });
}

double CurrencyConverter::convert(double amount, const std::string& from, const std::string& to) const {
    std::lock_guard<std::mutex> lock(rates_mutex_);

    if (from == to) return amount;
    if (rates_.empty()) throw std::runtime_error("Курсы валют не загружены");

    return amount * rates_.at(from) / rates_.at(to);
}

void CurrencyConverter::save_rates_to_file(const std::string& path) {
    std::lock_guard<std::mutex> lock(rates_mutex_);
    nlohmann::json j;
    for (const auto& [code, rate] : rates_) {
        j[code] = rate;
    }

    std::ofstream file(path);
    if (file) {
        file << j.dump(4);
    }
}

bool CurrencyConverter::load_rates_from_file(const std::string& path) {
    std::lock_guard<std::mutex> lock(rates_mutex_);
    std::ifstream file(path);
    if (!file) return false;

    try {
        nlohmann::json j;
        file >> j;

        rates_.clear();
        for (auto& [key, value] : j.items()) {
            rates_[key] = value.get<double>();
        }
        return true;
    }
    catch (...) {
        return false;
    }
}