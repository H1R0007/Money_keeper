#include "CurrencyFetcher.hpp"
#include "../libs/json.hpp"
#include "curl/CurlHttpClient.hpp"
#include <stdexcept>
#include <iostream>

using json = nlohmann::json;

void CurrencyFetcher::fetch_rates(RatesCallback callback) {
    const std::string url = "https://www.cbr-xml-daily.ru/daily_json.js";

    CurlHttpClient::Get(url, [this, callback](const std::string& json, bool success) {
        std::unordered_map<std::string, double> rates;
        if (!success) {
            std::cerr << "[ERROR] Ошибка загрузки курсов валют\n";
            callback({}); // Возвращаем пустой список
            return;
        }

        if (success) {
            parse_json(json, rates);
        }
        callback(rates);
        });
}

bool CurrencyFetcher::parse_json(const std::string& json_str, std::unordered_map<std::string, double>& rates) {
    try {
        auto data = json::parse(json_str);

        if (!data.contains("Valute")) {
            return false;
        }

        // Основные валюты
        for (const auto& item : data["Valute"]) {
            std::string code = item["CharCode"];
            double rate = item["Value"].get<double>() / item["Nominal"].get<double>();
            rates[code] = rate;
        }

        // Добавляем рубли
        rates["RUB"] = 1.0;
        return true;
    }
    catch (const std::exception& e) {
        return false;
    }
}
