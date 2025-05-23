/**
 * @file CurrencyFetcher.hpp
 * @brief Интерфейс загрузчика курсов валют
 *
 * @details Предоставляет:
 * - Асинхронный API для получения курсов
 * - Коллбэк-интерфейс для обработки результатов
 * - Внутренний парсер JSON-ответов
 *
 * @section dependencies Зависимости
 * Требует наличия:
 * - HTTP-клиента (CurlHttpClient)
 * - JSON-парсера (nlohmann/json)
 */

#pragma once
#include <string>
#include <unordered_map>
#include <functional>

 /**
  * @class CurrencyFetcher
  * @brief Класс для получения курсов валют с внешнего API
  *
  * @note Не поддерживает кэширование, повторные запросы
  * и обработку изменений курсов со временем
  */
class CurrencyFetcher {
public:
    /// Тип callback для возврата результатов
    using RatesCallback = std::function<void(const std::unordered_map<std::string, double>&)>;

    /**
     * @brief Инициирует загрузку курсов валют
     * @param callback Функция для обработки результатов
     *
     * @details Передает в callback:
     * - Словарь с курсами (код валюты -> курс к RUB)
     * - Пустой словарь в случае ошибки
     */
    void fetch_rates(RatesCallback callback);

private:
    /**
     * @brief Внутренний метод парсинга JSON
     * @param json Строка с JSON-данными
     * @param rates Ссылка на словарь для результатов
     * @return true если парсинг успешен
     *
     * @note Логика обработки:
     * - Извлекает коды валют (CharCode)
     * - Вычисляет курс как Value/Nominal
     * - Добавляет RUB с курсом 1.0
     */
    bool parse_json(const std::string& json, std::unordered_map<std::string, double>& rates);
};