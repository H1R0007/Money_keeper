/**
 * @file CurrencyFetcher.cpp
 * @brief ���������� ���������� ������ �����
 *
 * @details ������������:
 * - �������� ������ � ���������� API
 * - ������� JSON-������
 * - ������������ ������ �����
 * - ��������� ������ ���� � ��������
 *
 * @section data_source �������� ������
 * ���������� ��������� API �� �� �� ������:
 * https://www.cbr-xml-daily.ru/daily_json.js
 */

#include "CurrencyFetcher.hpp"
#include "../libs/json.hpp"
#include "curl/CurlHttpClient.hpp"
#include <stdexcept>
#include <iostream>

using json = nlohmann::json;

/**
 * @brief ��������� ���������� ����� �����
 * @param callback ������� ��������� ������ ��� �����������
 *
 * @details �������� ������:
 * 1. ��������� HTTP-������ � API �� ��
 * 2. ��� ��������� ������:
 *    - � ������ ������ ������ JSON
 *    - ����������� ����� (����� �� �������)
 *    - ��������� RUB � ������ 1.0
 *    - �������� ��������� � callback
 * 3. � ������ ������ �������� ������ ���������
 *
 * @note ��� ������ ���������� � stderr
 */
void CurrencyFetcher::fetch_rates(RatesCallback callback) {
    const std::string url = "https://www.cbr-xml-daily.ru/daily_json.js";

    CurlHttpClient::Get(url, [this, callback](const std::string& json, bool success) {
        std::unordered_map<std::string, double> rates;
        if (!success) {
            std::cerr << "[ERROR] ������ ��������� ������ �����\n";
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
 * @brief ������ JSON � ������� �����
 * @param json_str ������ � JSON-�������
 * @param rates ������ �� ������� ��� �����������
 * @return true ���� ������� �������
 *
 * @details ������� JSON � �������:
 * {
 *   "Valute": {
 *     "USD": {"Value": 75.5, "Nominal": 1},
 *     "EUR": {"Value": 89.2, "Nominal": 1},
 *     ...
 *   }
 * }
 *
 * @note ������������� ��������� RUB � ������ 1.0
 */
bool CurrencyFetcher::parse_json(const std::string& json_str, std::unordered_map<std::string, double>& rates) {
    try {
        auto data = json::parse(json_str);

        if (!data.contains("Valute")) {
            return false;
        }

        // ��������� ������ ������
        for (const auto& item : data["Valute"]) {
            std::string code = item["CharCode"];
            double rate = item["Value"].get<double>() / item["Nominal"].get<double>();
            rates[code] = rate;
        }

        // ���������� �����
        rates["RUB"] = 1.0;
        return true;
    }
    catch (const std::exception& e) {
        return false;
    }
}