/**
 * @file CurrencyConverter.cpp
 * @brief ���������� ���������� ����� � ���������� ���������� ������
 *
 * @details �������� �������:
 * - ���������� ������ ����� �� �������� ���������
 * - ����������� ���� ����� ��������
 * - ����������/�������� ������ � ����
 * - ���������������� �������� � �������
 *
 * @section data_handling ��������� ������
 * ���������� unordered_map ��� �������� ������ �����,
 * ��� ���� - ��� ������ (�������� "USD"), �������� - ���� � �����.
 * ��� �������� �������� ��������� ��� ������������������.
 */

#include "CurrencyConverter.hpp"
#include "CurrencyFetcher.hpp"
#include <../libs/json.hpp>
#include <fstream>
#include <filesystem>
#include <iostream>


 /**
  * @brief ��������� ����� ����� ����������
  * @param callback ������� ��������� ������, ����������� bool (�����/�������)
  *
  * @details �������� ������:
  * 1. ������� ������ CurrencyFetcher
  * 2. ��������� ����������� ��������� ������
  * 3. ��� ��������� ������:
  *    - ��������� �������
  *    - ��������� ���������� ��������� ������
  *    - �������� callback � �����������
  * 4. � ������ ������ �������� ������ ���������
  *
  * @note ���������������, ����� ���������� �� ������ ������
  */
void CurrencyConverter::update_rates(std::function<void(bool)> callback) {
    CurrencyFetcher fetcher;
    fetcher.fetch_rates([this, callback](const auto& new_rates) {
        std::lock_guard<std::mutex> lock(rates_mutex_);
        rates_ = new_rates;
        callback(!new_rates.empty());
        });
}

/**
 * @brief ������������ ����� ����� ��������
 * @param amount ����� ��� �����������
 * @param from �������� ������ (��� ISO)
 * @param to ������� ������ (��� ISO)
 * @return ��������� �����������
 *
 * @throws std::runtime_error ���� ����� �� ���������
 * @throws std::out_of_range ���� ������ �� �������
 *
 * @details ������� �����������:
 * result = amount * (rate_from / rate_to)
 * ��� rate_X - ���� ������ X � �����
 *
 * @note ���������������, ����� ���������� �� ������ ������
 */
double CurrencyConverter::convert(double amount, const std::string& from, const std::string& to) const {
    std::lock_guard<std::mutex> lock(rates_mutex_);

    if (from == to) return amount;
    if (rates_.empty()) throw std::runtime_error("����� ������");

    return amount * rates_.at(from) / rates_.at(to);
}

/**
 * @brief ��������� ������� ����� ����� � JSON-����
 * @param path ���� � ����� ��� ����������
 *
 * @details ������ �����:
 * {
 *   "USD": 75.45,
 *   "EUR": 89.12,
 *   ...
 * }
 *
 * @note ���������������, ��������� ������� �� ����� ��������
 */
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

/**
 * @brief ��������� ����� ����� �� JSON-�����
 * @param path ���� � ����� � �������
 * @return true ���� �������� �������, false ��� ������
 *
 * @details ������� ���� � ��� �� �������, ��� � save_rates_to_file
 * � ������ ������ �������� ��� ������ ����� ���������� false,
 * ��� ���� ������������ ����� �������� �����������
 *
 * @note ���������������, ��������� ������� �� ����� ��������
 */
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