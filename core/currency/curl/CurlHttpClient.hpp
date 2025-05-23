/**
 * @file CurlHttpClient.hpp
 * @brief HTTP-������ �� ������ libcurl
 *
 * @details ���������:
 * - ����������� GET-�������
 * - SSL/TLS ����������
 * - �������� ����������
 * - ��������� ������
 *
 * @section design_sec ������������� �������
 * 1. ����������� ����� ��� ���������
 * 2. Callback-��������� ��� �������������
 * 3. ��������� ������ HTTPS
 * 4. ������� �������� ������������
 */

#pragma once
#include <string>
#include <functional>

 /**
  * @class CurlHttpClient
  * @brief ������� ��� libcurl C API
  *
  * @warning ��������� ��������������� ������������� libcurl (curl_global_init)
  * @note ��������������� ��� ������� ����������� ������������� libcurl
  */

class CurlHttpClient {
public:
    /// ��� callback ��� ��������� ������
    using ResponseCallback = std::function<void(const std::string&, bool)>;

    /**
     * @brief ��������� ����������� HTTP GET ������
     * @param url ������ URL ������� (������� https://)
     * @param callback ������� ��������� ������
     * @return true ���� ������ ����� �����������
     *
     * @details ����������� ����������:
     * 1. ������������� ������� 5 ������
     * 2. �������� �������� SSL ������������
     * 3. ���������� ��������� ��������� ������������
     *
     * @throws �� ����������� ����������, ������ ���������� � callback
     */
    static bool Get(const std::string& url, ResponseCallback callback);

private:

    /**
     * @brief Callback ��� ������ ������
     * @param contents ���������� ������
     * @param size ������ �������� ������
     * @param nmemb ���������� ���������
     * @param output ��������� �� ������ ��� ���������� ������
     * @return ���������� ������������ ������ ������
     *
     * @note ������������ libcurl ��� ������������ ���������� ������
     */
    static size_t write_callback(void* contents, size_t size, size_t nmemb, std::string* output);
};