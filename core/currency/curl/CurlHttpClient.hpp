/**
 * @file CurlHttpClient.hpp
 * @brief HTTP-клиент на основе libcurl
 *
 * @details Реализует:
 * - Асинхронные GET-запросы
 * - SSL/TLS шифрование
 * - Таймауты соединения
 * - Обработку ошибок
 *
 * @section design_sec Архитектурные решения
 * 1. Статический класс без состояния
 * 2. Callback-интерфейс для асинхронности
 * 3. Поддержка только HTTPS
 * 4. Строгая проверка сертификатов
 */

#pragma once
#include <string>
#include <functional>

 /**
  * @class CurlHttpClient
  * @brief Обертка для libcurl C API
  *
  * @warning Требуется предварительная инициализация libcurl (curl_global_init)
  * @note Потокобезопасна при условии корректного использования libcurl
  */

class CurlHttpClient {
public:
    /// Тип callback для обработки ответа
    using ResponseCallback = std::function<void(const std::string&, bool)>;

    /**
     * @brief Выполняет асинхронный HTTP GET запрос
     * @param url Полный URL запроса (включая https://)
     * @param callback Функция обратного вызова
     * @return true если запрос начал выполняться
     *
     * @details Особенности реализации:
     * 1. Устанавливает таймаут 5 секунд
     * 2. Включает проверку SSL сертификатов
     * 3. Использует системное хранилище сертификатов
     *
     * @throws Не выбрасывает исключений, ошибки передаются в callback
     */
    static bool Get(const std::string& url, ResponseCallback callback);

private:

    /**
     * @brief Callback для записи ответа
     * @param contents Полученные данные
     * @param size Размер элемента данных
     * @param nmemb Количество элементов
     * @param output Указатель на строку для накопления ответа
     * @return Фактически обработанный размер данных
     *
     * @note Используется libcurl для постепенного накопления ответа
     */
    static size_t write_callback(void* contents, size_t size, size_t nmemb, std::string* output);
};