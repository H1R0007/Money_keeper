/**
 * @file CurrencyConverter.hpp
 * @brief ������������ ���� ��� ������ ���������� �����
 *
 * @details ������������� ��������� ���:
 * - ����������� �����
 * - ���������� ����� ������ �����
 * - ����������/�������� ������
 * - �������� ��������� �����
 *
 * @section thread_safety ������������������
 * ��� ��������� ������ ��������������� ��������� ����������� ��������.
 * ������������ ������������ ������ �� ������ �������.
 */

#pragma once
#include <string>
#include <unordered_map>
#include <mutex>
#include <functional>

 /**
  * @class CurrencyConverter
  * @brief �������� ����� ��� ������ � ��������
  *
  * @details ���������� ������� API ��� ��������� ���������� ������ �����
  * � ������������� ���������� ��� �� �����������. ��� ����� ��������
  * ������������ ����������� ����� (RUB).
  */

class CurrencyConverter {
public:
    /**
     * @brief ���������� ��������� ����� �����
     * @param callback ������� ��������� ������, ����������� bool (����� ��������)
     *
     * @note ������� callback � false ���� �� ������� �������� ����� �����
     */

    void update_rates(std::function<void(bool)> callback);

    /**
     * @brief ������������ ����� ����� ��������
     * @param amount ����� ��� �����������
     * @param from �������� ������ (3-��������� ��� ISO)
     * @param to ������� ������ (�� ��������� RUB)
     * @return ��������� �����������
     *
     * @throws std::runtime_error ���� ����� �� ���������
     * @throws std::out_of_range ���� ������ �� �������
     */
    double convert(double amount, const std::string& from, const std::string& to = "RUB") const;

    /**
     * @brief ��������� ��������� ������
     * @param currency_code 3-��������� ��� ������ (ISO)
     * @return true ���� ������ �������� ��� �����������
     *
     * @note ����� �� ���������������, ������ ���������� ��� ��������������� ��������
     */
    bool is_currency_supported(const std::string& currency_code) const {
        //std::lock_guard<std::mutex> lock(rates_mutex_);
        return rates_.find(currency_code) != rates_.end();
    }

    /**
     * @brief ��������� ������� ����� � ����
     * @param path ���� � ����� ��� ����������
     *
     * @details ��������� � ������� JSON � ��������� (pretty print)
     */
    void save_rates_to_file(const std::string& path);

    /**
     * @brief ��������� ����� �� �����
     * @param path ���� � ����� � �������
     * @return true ���� �������� �������
     */
    bool load_rates_from_file(const std::string& path);

    /**
     * @brief ������������� ����� ����� �������
     * @param new_rates ����� ����� ����� (��� -> ���� � RUB)
     *
     * @warning ������������ ������ ��� ������������!
     * � production-���� ����������� update_rates()
     */
    void set_rates(const std::unordered_map<std::string, double>& new_rates) {
        std::lock_guard<std::mutex> lock(rates_mutex_);
        rates_ = new_rates;
    }

private:
    std::unordered_map<std::string, double> rates_; ///< ��������� ������ ����� (��� -> ���� � RUB)
    mutable std::mutex rates_mutex_; ///< ������� ��� ������ ������� � rates_
};