/**
 * @file CurrencyFetcher.cpp
 * @brief Реализация загрузчика курсов валют
 *
 * @details Осуществляет:
 * - Загрузку данных с удаленного API
 * - Парсинг JSON-ответа
 * - Нормализацию курсов валют
 * - Обработку ошибок сети и парсинга
 *
 * @section data_source Источник данных
 * Использует публичное API ЦБ РФ по адресу:
 * https://www.cbr-xml-daily.ru/daily_json.js
 */

#include "CurrencyFetcher.hpp"
#include "../libs/json.hpp"
#include "curl/CurlHttpClient.hpp"
#include <stdexcept>
#include <iostream>

using json = nlohmann::json;

/**
 * @brief Загружает актуальные курсы валют
 * @param callback Функция обратного вызова для результатов
 *
 * @details Алгоритм работы:
 * 1. Формирует HTTP-запрос к API ЦБ РФ
 * 2. При получении ответа:
 *    - В случае успеха парсит JSON
 *    - Нормализует курсы (делит на номинал)
 *    - Добавляет RUB с курсом 1.0
 *    - Передает результат в callback
 * 3. В случае ошибки передает пустой результат
 *
 * @note Все ошибки логируются в stderr
 */
void CurrencyFetcher::fetch_rates(RatesCallback callback) {
    const std::string url = "https://www.cbr-xml-daily.ru/daily_json.js";

    CurlHttpClient::Get(url, [this, callback](const std::string& json, bool success) {
        std::unordered_map<std::string, double> rates;
        if (!success) {
            std::cerr << "[ERROR] Ошибка получения курсов валют\n";
            callback({});
            return;
        }

        if (success) {
            parse_json(json, rates);
        }
        callback(rates);
        });
}

/**
 * @brief Парсит JSON с курсами валют
 * @param json_str Строка с JSON-данными
 * @param rates Ссылка на словарь для результатов
 * @return true если парсинг успешен
 *
 * @details Ожидает JSON в формате:
 * {
 *   "Valute": {
 *     "USD": {"Value": 75.5, "Nominal": 1},
 *     "EUR": {"Value": 89.2, "Nominal": 1},
 *     ...
 *   }
 * }
 *
 * @note Автоматически добавляет RUB с курсом 1.0
 */
bool CurrencyFetcher::parse_json(const std::string& json_str, std::unordered_map<std::string, double>& rates) {
    try {
        auto data = json::parse(json_str);

        if (!data.contains("Valute")) {
            return false;
        }

        // Обработка каждой валюты
        for (const auto& item : data["Valute"]) {
            std::string code = item["CharCode"];
            double rate = item["Value"].get<double>() / item["Nominal"].get<double>();
            rates[code] = rate;
        }

        // Добавление рубля
        rates["RUB"] = 1.0;
        return true;
    }
    catch (const std::exception& e) {
        return false;
    }
}