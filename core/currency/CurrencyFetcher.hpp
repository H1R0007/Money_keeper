/**
 * @file CurrencyFetcher.hpp
 * @brief ��������� ���������� ������ �����
 *
 * @details �������������:
 * - ����������� API ��� ��������� ������
 * - �������-��������� ��� ��������� �����������
 * - ���������� ������ JSON-�������
 *
 * @section dependencies �����������
 * ������� �������:
 * - HTTP-������� (CurlHttpClient)
 * - JSON-������� (nlohmann/json)
 */

#pragma once
#include <string>
#include <unordered_map>
#include <functional>

 /**
  * @class CurrencyFetcher
  * @brief ����� ��� ��������� ������ ����� � �������� API
  *
  * @note �� ������������ �����������, ��������� �������
  * � ��������� ��������� ������ �� ��������
  */
class CurrencyFetcher {
public:
    /// ��� callback ��� �������� �����������
    using RatesCallback = std::function<void(const std::unordered_map<std::string, double>&)>;

    /**
     * @brief ���������� �������� ������ �����
     * @param callback ������� ��� ��������� �����������
     *
     * @details �������� � callback:
     * - ������� � ������� (��� ������ -> ���� � RUB)
     * - ������ ������� � ������ ������
     */
    void fetch_rates(RatesCallback callback);

private:
    /**
     * @brief ���������� ����� �������� JSON
     * @param json ������ � JSON-�������
     * @param rates ������ �� ������� ��� �����������
     * @return true ���� ������� �������
     *
     * @note ������ ���������:
     * - ��������� ���� ����� (CharCode)
     * - ��������� ���� ��� Value/Nominal
     * - ��������� RUB � ������ 1.0
     */
    bool parse_json(const std::string& json, std::unordered_map<std::string, double>& rates);
};