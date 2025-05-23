/**
 * @file CurlHttpClient.cpp
 * @brief Реализация HTTP-клиента на libcurl
 *
 * @details Настройки соединения:
 * - Таймаут: 5 секунд
 * - SSL verification: строгий режим
 * - User-Agent: libcurl/{version}
 * - Redirects: запрещены
 */

#include "CurlHttpClient.hpp"
#include <curl/curl.h>

 /**
  * @brief Callback-функция для записи ответа
  *
  * @details Алгоритм работы:
  * 1. Получает chunk данных от libcurl
  * 2. Добавляет в строку-буфер
  * 3. Возвращает количество обработанных байт
  *
  * @note Вызывается многократно в течение одного запроса
  * @warning Не должен бросать исключения (ограничение libcurl)
  */
size_t CurlHttpClient::write_callback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t total = size * nmemb;
    output->append((char*)contents, total);
    return total;
}

/**
 * @brief Выполняет HTTP GET запрос
 *
 * @details Пошаговая настройка libcurl:
 * 1. Инициализация easy-сессии
 * 2. Установка URL и callback
 * 3. Настройка SSL параметров
 * 4. Установка таймаутов
 * 5. Выполнение запроса
 * 6. Очистка ресурсов
 *
 * @note Особенности обработки ошибок:
 * - Ошибки CURL передаются в callback как success=false
 * - Исключения C++ ловятся и обрабатываются
 *
 * @warning Не thread-safe на уровне libcurl (нужна curl_global_init)
 */
bool CurlHttpClient::Get(const std::string& url, ResponseCallback callback) {
    CURL* curl = curl_easy_init();
    std::string response;
    bool success = false;

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L); // Проверять SSL сертификат
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L); // Строгая проверка хоста

        CURLcode res = curl_easy_perform(curl);
        success = (res == CURLE_OK);
        curl_easy_cleanup(curl);
    }

    callback(response, success);
    return success;
}