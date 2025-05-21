/**
 * @file CurrencyConverter.cpp
 * @brief Реализация конвертера валют с поддержкой обновления курсов
 *
 * @details Основные функции:
 * - Обновление курсов валют из внешнего источника
 * - Конвертация сумм между валютами
 * - Сохранение/загрузка курсов в файл
 * - Потокобезопасные операции с курсами
 *
 * @section data_handling Обработка данных
 * Использует unordered_map для хранения курсов валют,
 * где ключ - код валюты (например "USD"), значение - курс к рублю.
 * Все операции защищены мьютексом для потокобезопасности.
 */

#include "CurrencyConverter.hpp"
#include "CurrencyFetcher.hpp"
#include <../libs/json.hpp>
#include <fstream>
#include <filesystem>
#include <iostream>


 /**
  * @brief Обновляет курсы валют асинхронно
  * @param callback Функция обратного вызова, принимающая bool (успех/неудача)
  *
  * @details Алгоритм работы:
  * 1. Создает объект CurrencyFetcher
  * 2. Запускает асинхронное получение курсов
  * 3. При получении ответа:
  *    - Блокирует мьютекс
  *    - Обновляет внутреннее хранилище курсов
  *    - Вызывает callback с результатом
  * 4. В случае ошибки передает пустой результат
  *
  * @note Потокобезопасно, может вызываться из любого потока
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
 * @brief Конвертирует сумму между валютами
 * @param amount Сумма для конвертации
 * @param from Исходная валюта (код ISO)
 * @param to Целевая валюта (код ISO)
 * @return Результат конвертации
 *
 * @throws std::runtime_error если курсы не загружены
 * @throws std::out_of_range если валюта не найдена
 *
 * @details Формула конвертации:
 * result = amount * (rate_from / rate_to)
 * Где rate_X - курс валюты X к рублю
 *
 * @note Потокобезопасно, может вызываться из любого потока
 */
double CurrencyConverter::convert(double amount, const std::string& from, const std::string& to) const {
    std::lock_guard<std::mutex> lock(rates_mutex_);

    if (from == to) return amount;
    if (rates_.empty()) throw std::runtime_error("����� ����� �� ���������");

    return amount * rates_.at(from) / rates_.at(to);
}

/**
 * @brief Сохраняет текущие курсы валют в JSON-файл
 * @param path Путь к файлу для сохранения
 *
 * @details Формат файла:
 * {
 *   "USD": 75.45,
 *   "EUR": 89.12,
 *   ...
 * }
 *
 * @note Потокобезопасно, блокирует мьютекс на время операции
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
 * @brief Загружает курсы валют из JSON-файла
 * @param path Путь к файлу с курсами
 * @return true если загрузка успешна, false при ошибке
 *
 * @details Ожидает файл в том же формате, что и save_rates_to_file
 * В случае ошибки парсинга или чтения файла возвращает false,
 * при этом существующие курсы остаются неизменными
 *
 * @note Потокобезопасно, блокирует мьютекс на время операции
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