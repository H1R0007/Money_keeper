/**
 * @file CurrencyConverter.hpp
 * @brief Заголовочный файл для класса конвертера валют
 *
 * @details Предоставляет интерфейс для:
 * - Конвертации валют
 * - Управления кэшем курсов валют
 * - Сохранения/загрузки курсов
 * - Проверки поддержки валют
 *
 * @section thread_safety Потокобезопасность
 * Все публичные методы потокобезопасны благодаря внутреннему мьютексу.
 * Поддерживает конкурентные вызовы из разных потоков.
 */

#pragma once
#include <string>
#include <unordered_map>
#include <mutex>
#include <functional>

 /**
  * @class CurrencyConverter
  * @brief Основной класс для работы с валютами
  *
  * @details Использует внешний API для получения актуальных курсов валют
  * и предоставляет функционал для их конвертации. Все курсы хранятся
  * относительно российского рубля (RUB).
  */

class CurrencyConverter {
public:
    /**
     * @brief Асинхронно обновляет курсы валют
     * @param callback Функция обратного вызова, принимающая bool (успех операции)
     *
     * @note Вызовет callback с false если не удалось получить новые курсы
     */

    void update_rates(std::function<void(bool)> callback);

    /**
     * @brief Конвертирует сумму между валютами
     * @param amount Сумма для конвертации
     * @param from Исходная валюта (3-буквенный код ISO)
     * @param to Целевая валюта (по умолчанию RUB)
     * @return Результат конвертации
     *
     * @throws std::runtime_error если курсы не загружены
     * @throws std::out_of_range если валюта не найдена
     */
    double convert(double amount, const std::string& from, const std::string& to = "RUB") const;

    /**
     * @brief Проверяет поддержку валюты
     * @param currency_code 3-буквенный код валюты (ISO)
     * @return true если валюта доступна для конвертации
     *
     * @note Метод не потокобезопасен, должен вызываться при заблокированном мьютексе
     */
    bool is_currency_supported(const std::string& currency_code) const {
        //std::lock_guard<std::mutex> lock(rates_mutex_);
        return rates_.find(currency_code) != rates_.end();
    }

    /**
     * @brief Сохраняет текущие курсы в файл
     * @param path Путь к файлу для сохранения
     *
     * @details Сохраняет в формате JSON с отступами (pretty print)
     */
    void save_rates_to_file(const std::string& path);

    /**
     * @brief Загружает курсы из файла
     * @param path Путь к файлу с курсами
     * @return true если загрузка успешна
     */
    bool load_rates_from_file(const std::string& path);

    /**
     * @brief Устанавливает курсы валют вручную
     * @param new_rates Новые курсы валют (код -> курс к RUB)
     *
     * @warning Предназначен только для тестирования!
     * В production-коде используйте update_rates()
     */
    void set_rates(const std::unordered_map<std::string, double>& new_rates) {
        std::lock_guard<std::mutex> lock(rates_mutex_);
        rates_ = new_rates;
    }

private:
    std::unordered_map<std::string, double> rates_; ///< Хранилище курсов валют (код -> курс к RUB)
    mutable std::mutex rates_mutex_; ///< Мьютекс для защиты доступа к rates_
};